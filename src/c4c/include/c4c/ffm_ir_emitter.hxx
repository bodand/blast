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
 * Originally created: 2025-08-08.
 *
 * src/c4c/include/c4c/ffm_ir_emitter --
 *   
 */
#ifndef BLAST_FFM_IR_EMITTER_HXX
#define BLAST_FFM_IR_EMITTER_HXX

#include <string_view>

#include <c4/ffm/fwd.hxx>

#include <c4rt2c/c4rt_emitter.hxx>

#include <llvm/IR/PassManager.h>

namespace llvm {
    class Module;
    class LLVMContext;
    class Function;
    class Type;
    class FunctionType;
    class Value;
    class BasicBlock;
    class ConstantFolder;
    class IRBuilderDefaultInserter;
    template<typename FolderTy, typename InserterTy>
    class IRBuilder;
}

namespace c4c {
    struct ffm_ir_emitter final : c4::ffm::ffm_visitor {
        ffm_ir_emitter(llvm::LLVMContext& context,
                       llvm::Module& module,
                       llvm::IRBuilder<llvm::ConstantFolder, llvm::IRBuilderDefaultInserter>& builder,
                       llvm::FunctionPassManager& pass_manager,
                       llvm::FunctionAnalysisManager& fna_manager);

        void do_visit(const c4::ffm::block_argument& obj) override;

        void do_visit(const c4::ffm::context_type& obj) override;

        void do_visit(const c4::ffm::context_object& obj) override;

        void do_visit(const c4::ffm::context_access& obj) override;

        void do_visit(const c4::ffm::root_expression& obj) override;

        void do_visit(const c4::ffm::value_expression& obj) override;

        void do_visit(const c4::ffm::function_call& obj) override;

        void do_visit(const c4::ffm::dynamic_call& obj) override;

        void do_visit(const c4::ffm::function_declaration& obj) override;

        void do_visit(const c4::ffm::function_definition& obj) override;

        void do_visit(const c4::ffm::literal& obj) override;

        void do_visit(const c4::ffm::block_literal& obj) override;

        void do_visit(const c4::ffm::local& obj) override;

        void do_visit(const c4::ffm::local_ref& obj) override;

        void do_visit(const c4::ffm::unpack& obj) override;

        void do_visit(const c4::ffm::function& obj) override;

        llvm::Function* entry;

    private:
        struct call_stack {
            call_stack(bool& outer_call,
                       std::vector<llvm::Value*>& outer_args)
                : _outer_call{outer_call}
                , _outer_args{outer_args}
                , active_call{outer_call}
                , call_args{std::move(outer_args)} {
                outer_call = true;
                outer_args.clear();
            }

            void
            pop() const {
                if (popped) return;
                popped = true;
                _outer_call = active_call;
                _outer_args = std::move(call_args);
            }

            ~call_stack() { pop(); }

            bool& _outer_call;
            std::vector<llvm::Value*>& _outer_args;
            bool active_call;
            std::vector<llvm::Value*> call_args{};

        private:
            mutable bool popped{false};
        };

        void
        push_value(llvm::Value* value);

        llvm::Value*
        build_literal(const c4::ffm::literal& lit);

        llvm::FunctionType*
        build_type_for(const c4::ffm::function_declaration& decl) const;

        llvm::FunctionType*
        build_type_with_arity(unsigned arity) const;

        [[nodiscard]] llvm::BasicBlock*
        build_bblock(std::string_view name) const;

        llvm::FunctionPassManager& _pass_manager;
        llvm::FunctionAnalysisManager& _fna_manager;
        llvm::Function* _current_function{};

        bool _in_unpack{false};
        bool _returned_value{false};
        bool _active_call{false};
        std::vector<llvm::Value*> _current_args{};

        llvm::LLVMContext& _context;
        llvm::Module& _module;
        llvm::IRBuilder<llvm::ConstantFolder, llvm::IRBuilderDefaultInserter>& _builder;
        llvm::Type* _c4rt_datum_type;
        llvm::Type* _c4rt_package_type;

        c4rt2c::c4_rt2_emitter _rt_emitter;
    };
}

#endif
