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
 * src/c4/include/c4/ffm_mapper --
 *   
 */
#ifndef BLAST_FFM_MAPPER_HXX
#define BLAST_FFM_MAPPER_HXX

#include <deque>
#include <optional>
#include <span>
#include <vector>

#include <c4/ffm/ffm_context.hxx>
#include <c4/visitor/visitor.hxx>
#include <fmt/ranges.h>

#include <c4/ast2/symbol.hxx>

namespace c4::ast2 {
    struct block;
    struct block_args;
    struct let_expression;
    struct expression;
    struct fn_call;
    struct binary_op_call;
    struct unary_op_call;
    struct float_literal;
    struct integer_literal;
    struct string_literal;
    struct dynamic_call;
}

namespace c4 {
    //
    namespace ffm {
        struct argument_holder;
    }

    template<class T>
    struct recursive_scope {
        explicit
        recursive_scope(T& counter, T temp = T()) noexcept
            : _original_value(counter)
            , _ref(counter) {
            counter = temp;
        }

        ~recursive_scope() noexcept { _ref = _original_value; }

    private:
        T _original_value;
        T& _ref;
    };

    template<class T>
    recursive_scope(T&) -> recursive_scope<T>;

    struct ffm_mapper final : ast2::visitor<
                ast2::block,
                ast2::expression,
                ast2::let_expression,
                ast2::fn_call,
                ast2::binary_op_call,
                ast2::unary_op_call,
                ast2::float_literal,
                ast2::integer_literal,
                ast2::string_literal,
                ast2::dynamic_call
            > {
        explicit ffm_mapper(diagnostics_engine& diag, ffm::ffm_context& ffm_context);

        void do_visit(const ast2::let_expression& obj) override;

        void do_visit(const ast2::block& obj) override;

        void do_visit(const ast2::expression& obj) override;

        void do_visit(const ast2::binary_op_call& obj) override;

        void do_visit(const ast2::unary_op_call& obj) override;

        void do_visit(const ast2::fn_call& obj) override;

        void do_visit(const ast2::float_literal& obj) override;

        void do_visit(const ast2::integer_literal& obj) override;

        void do_visit(const ast2::string_literal& obj) override;

        void do_visit(const ast2::dynamic_call& obj) override;

        void finalize_block_body() const;

        [[nodiscard]] const std::deque<ffm::function*>&
        roots() const noexcept { return _roots; }

    private:
        struct block_entry {
            explicit
            block_entry(std::size_t& counter) noexcept
                : _block_id(counter)
                , _counter(counter) {
                counter = 0;
            }

            ~block_entry() noexcept { _counter = _block_id; }

        private:
            std::size_t _block_id;
            std::size_t& _counter;
        };

        std::vector<ffm::block_argument*>
        make_argument_list(const ast2::block_args* args);

        [[nodiscard]] ffm::function_declaration*
        find_function_declaration(const ffm::symbol& sym) const;

        ffm::root_expression*
        build_root_literal(ffm::literal* lit) const;

        ffm::value_expression*
        build_value_literal(ffm::literal* lit) const;

        void
        push_literal(ffm::literal* ffm_lit);

        void
        push_block_literal(ffm::function_declaration* decl);

        void
        push_call_literal(ffm::literal* literal);

        void
        push_root_literal(ffm::literal* lit);

        void
        push_value_expression(ffm::value_expression* expr) const;

        void
        push_context_access(ffm::context_access* ctx_expr) const;

        void
        build_closure_context_from_symbols(const ast2::expression& obj);

        ffm::function_declaration*
        declare_function(const ffm::symbol&, std::vector<ffm::block_argument*>&& args);

        ffm::function_declaration*
        declare_extern_function(const ffm::symbol& sym);

        [[nodiscard("store as automatic variable")]] recursive_scope<ffm::argument_holder*>
        enter_call_arguments(ffm::argument_holder* call);

        ffm::root_expression*
        build_root_function_call(const position& position, const ast2::symbol& sym,
                                 std::span<const ast2::expression* const> args);

        ffm::function_declaration*
        resolve_function_declaration(const ast2::symbol& sym);

        ffm::function_call*
        build_function_pack_from_symbol(const position& position,
                                        const ast2::symbol& sym);

        ffm::function_call*
        build_function_call_from_symbol(const position& position,
                                        const ast2::symbol& sym);

        ffm::value_expression*
        build_packed_function_call(const position& position,
                                   const ast2::symbol& sym,
                                   std::span<const ast2::expression* const>);

        void
        push_call_argument_packed(const position& position,
                                  const ast2::symbol& sym,
                                  std::span<const ast2::expression* const> args);

        ffm::value_expression*
        build_pack_value_expression(const position& position,
                                    const ast2::symbol& sym,
                                    std::span<const ast2::expression* const> args);

        void
        push_local(const position& position,
                   const ast2::symbol& sym,
                   std::span<const ast2::expression* const> args);

        void
        push_local(ffm::literal* literal);

        ffm::unpack*
        build_argument_unpack(ffm::block_argument* arg);

        ffm::root_expression*
        build_root_argument(ffm::block_argument* arg);

        ffm::value_expression*
        build_value_argument(ffm::block_argument* arg) const;

        ffm::context_object*
        build_context_object(const ffm::function_declaration* decl);

        ffm::value_expression*
        build_context_object(const ast2::symbol& sym);

        void
        push_context_object(const ast2::symbol& sym);

        void
        push_call(const position& position,
                  const ast2::symbol& sym,
                  std::span<const ast2::expression* const> args);

        void
        push_root_call(const position& position,
                       const ast2::symbol& sym,
                       std::span<const ast2::expression* const> args);

        [[nodiscard("store as automatic variable")]] recursive_scope<ffm::function_definition*>
        define_function(const ffm::function_declaration* decl);

        [[nodiscard]] std::size_t
        get_next_block_id() { return _block_counter++; }

        [[nodiscard("store as automatic variable")]] recursive_scope<std::size_t>
        enter_block() noexcept { return recursive_scope(_block_counter); }

        [[nodiscard]] std::string
        mangled_scope(const std::string_view mangled) const {
            if (_block_names.empty()) return std::string(mangled);
            return fmt::format("N{}E{}", fmt::join(_block_names, ""), mangled);
        }

        void
        process_call_arguments(const ast2::symbol& sym,
                               std::span<const ast2::expression* const> args,
                               ffm::argument_holder* call);

        [[nodiscard]] std::string
        next_block_name(unsigned arity);

        diagnostics_engine& _diag;

        std::size_t _block_counter{0};
        std::vector<std::string_view> _block_names{};
        ffm::context_type* _closure{};
        std::optional<const ast2::let_expression*> _currently_in_let{};

        ffm::function_definition* _current_function{};
        ffm::argument_holder* _current_call{};

        std::deque<ffm::function*> _roots{};
        ffm::ffm_context& _ffm_context;
    };
}

#endif
