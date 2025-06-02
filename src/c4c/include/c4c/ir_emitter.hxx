/* blAST project
 *
 * Copyright (c) 2025 András Bodor <bodand@pm.me>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * - Neither the name of the copyright holder nor the names of its contributors
 *   may be used to endorse or promote products derived from this software
 *   without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Originally created: 2025-04-04.
 *
 * src/c4c/include/c4c/rt_emitter --
 *   
 */
#ifndef C4C_RT_EMITTER_HXX
#define C4C_RT_EMITTER_HXX

#include <vector>

#include <c4/ast2/expression.hxx>
#include <c4/visitor/visitor.hxx>

#include <c4c/ir_emitter_memento.hxx>

namespace llvm {
    class Module;
    class LLVMContext;
    class Function;
    class Value;
    class BasicBlock;
    class ConstantFolder;
    class IRBuilderDefaultInserter;
    template<typename FolderTy, typename InserterTy>
    class IRBuilder;
}

namespace c4::ast2 {
    struct undef_symbol;
    struct ast_context;
    struct binary_op_call;
    struct unary_op_call;
    struct fn_call;
    struct string_literal;
    struct integer_literal;
    struct float_literal;
    struct let_expression;
    struct block_args;
    struct block;
    struct expression;
}

namespace c4c {
    struct c4_runtime_emitter;

    struct ir_emitter final : c4::ast2::visitor<
                c4::ast2::block,
                c4::ast2::block_args,
                c4::ast2::let_expression,
                c4::ast2::float_literal,
                c4::ast2::integer_literal,
                c4::ast2::string_literal,
                c4::ast2::fn_call,
                c4::ast2::unary_op_call,
                c4::ast2::binary_op_call
            > {
        struct last_value {
            llvm::Value* value{};
            bool constant{};

            void
            set_expr(llvm::Value* value) {
                this->value = value;
                constant = false;
            }

            void
            set_constant(llvm::Value* value) {
                this->value = value;
                constant = true;
            }
        };

        ir_emitter(c4_runtime_emitter& rt_emitter,
                   c4::ast2::ast_context& ast_context,
                   llvm::LLVMContext& context,
                   llvm::Module& module,
                   llvm::IRBuilder<>& builder,
                   std::vector<c4::ast2::undef_symbol> const& promised_symbols);

        void
        do_visit(const c4::ast2::fn_call& obj) override;

        void
        do_visit(const c4::ast2::unary_op_call& obj) override;

        void
        do_visit(const c4::ast2::binary_op_call& obj) override;

        void
        do_visit(const c4::ast2::block_args& obj) override;

        void
        do_visit(const c4::ast2::block& obj) override;

        void
        do_visit(const c4::ast2::let_expression& obj) override;

        void
        do_visit(const c4::ast2::float_literal& obj) override;

        void
        do_visit(const c4::ast2::string_literal& obj) override;

        void
        do_visit(const c4::ast2::integer_literal& obj) override;

        void
        finalize();

        ~ir_emitter() noexcept override;

        llvm::Function* entry;
        last_value last{};
        c4_runtime_emitter& rt_emitter;
        c4::ast2::ast_context& ast_context;
        llvm::LLVMContext& context;
        llvm::Module& module;
        llvm::IRBuilder<llvm::ConstantFolder, llvm::IRBuilderDefaultInserter>& builder;

    private:
        void
        initialize_closure_context_storage(llvm::Value* context_storage,
                                           std::span<const c4::ast2::symbol* const> symbols) const;

        llvm::Value*
        create_closure_context(std::span<const c4::ast2::symbol* const> effective_closure_symbols);

        std::vector<const c4::ast2::symbol*>
        filter_closure_symbols(const c4::ast2::expression& obj);

        llvm::Value*
        create_effective_closure(const c4::ast2::expression& obj);

        bool
        emit_closure_context(const c4::ast2::let_expression& obj);

        bool
        is_skipped_in_context(const c4::ast2::symbol& sym);

        llvm::Value*
        try_materialize_promise(std::string_view sym);

        llvm::Value*
        lookup(const c4::ast2::symbol& sym);

        [[deprecated]] llvm::Value*
        lookup_symbol(std::string_view sym);

        void
        build_return();

        struct scope_override_fixer {
            void
            add_symbol(c4::ast2::tags::referable* sym, llvm::Value* new_value);

            void
            operator()();

        private:
            struct scope_override {
                c4::ast2::tags::referable* symbol;
                std::unique_ptr<c4::ast2::tags::attribute> old;
            };

            std::vector<scope_override> _overridden{};
        };

        void
        emit_fn_body_from_block(const c4::ast2::block& obj);

        void
        emit_context_expansion(scope_override_fixer& fixer, const c4::ast2::block& obj);

        void
        generate_cleanup(llvm::BasicBlock* fn_body,
                         std::span<llvm::Value*const> cleanup) const;

        friend struct ir_emitter_memento;

        scoped_memento
        save_state();

        template<class Fn>
        void
        enter_function_emission(llvm::Function* fn,
                                const bool closure,
                                const c4::ast2::expression& val,
                                Fn&& emitter) {
            const auto mem = save_state();

            _active_function = fn;
            _function_is_closure = closure;

            const auto fn_body = build_bblock("body");
            std::invoke(std::forward<Fn>(emitter), val);

            generate_cleanup(fn_body, _need_cleanup);
        }

        [[nodiscard]] llvm::BasicBlock*
        build_bblock(std::string_view name, bool set_insert = true) const;

        llvm::Type* _arg_type;
        bool _finalized{false};
        bool _function_is_closure{false};
        std::string _block_name{};
        llvm::Function* _active_function{};
        std::vector<llvm::Value*> _need_cleanup{};
        std::unordered_map<std::string, llvm::Value*> _loaded_promised_symbols;
        std::vector<c4::ast2::undef_symbol> _promised_symbols;
        std::vector<llvm::BasicBlock*> _orphan_blocks{};
    };
}

#endif
