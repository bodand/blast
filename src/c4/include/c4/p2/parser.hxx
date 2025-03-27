/* demo project
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
 * Originally created: 2025-03-03.
 *
 * src/c4/include/c4/p2/parser --
 *   
 */
#ifndef C4_P2_PARSER_HXX
#define C4_P2_PARSER_HXX

#include <stdexcept>
#include <expected>
#include <vector>

#include <c4/ast2/symbol.hxx>
#include <c4/ast2/string_literal.hxx>
#include <c4/ast2/integer_literal.hxx>
#include <c4/ast2/float_literal.hxx>

#include <c4/p2/lex/lexer.hxx>
#include <utility>
#include <c4/ast2/expression.hxx>
#include <c4/ast2/let_expression.hxx>

namespace c4::p2 {
    struct bad_token_error final : std::runtime_error {
        bad_token_error()
            : std::runtime_error("parser failure: bad token") { }
    };

    namespace aux {
        template<class T>
        struct token_selector {
            std::expected<T, source_diagnostic>
            operator()(const T& tok) const { return tok; }

            template<class Found>
            std::expected<T, source_diagnostic>
            operator()(const Found& tok) const {
                return std::unexpected(source_diagnostic::error_for_value(
                    fmt::format(R"(expected "{}" but found "{}")",
                                T::token_name, Found::token_name),
                    tok.source_name(),
                    tok.token_position(),
                    tok.value()
                ));
            }
        };
    }

    struct parser {
        explicit
        parser(lexer&& lexer)
            : _lexer{std::move(lexer)} {
            next_relevant();
            enter_scope();
        }

        ast2::integer_literal
        parse_integer_literal();

        ast2::float_literal
        parse_float_literal();

        ast2::string_literal
        parse_string_literal();

        ast2::symbol
        parse_symbol();

        ast2::op_symbol
        parse_op_symbol();

        ast2::symbol
        parse_bare_symbol();

        ast2::expression
        parse_expression();

        ast2::let_expression
        parse_let_expression();

        ast2::expression
        parse_final_expression();

        ast2::block
        parse_block();

        ast2::block_args
        parse_block_args();

        std::vector<ast2::expression>
        parse_script();

        void
        declare_symbol(std::string_view symbol, unsigned arity);

        void
        declare_binop(std::string_view symbol, unsigned precedence, bool right_assoc);

        void
        declare_uniop(std::string_view symbol);

        [[nodiscard]] bool
        valid() const noexcept { return _valid; }

        std::vector<ast2::undef_symbol>
        promised_symbols() const;

    private:
        void
        parse_n_expressions(unsigned n,
                            std::vector<ast2::expression>& expressions,
                            std::vector<ast2::symbol>& closure_symbols);

        void
        reresolve_childs_closure_symbols(const ast2::expression& expr,
                                         std::vector<ast2::symbol>& closure_symbols);

        ast2::expression
        parse_operator_precedence(ast2::expression&& lhs, unsigned precedence);

        struct parser_symbol {
            std::string_view name;
            unsigned arity;

            parser_symbol(const std::string_view& name,
                          const unsigned arity)
                : name{name}
                , arity{arity} { }

            bool
            operator==(const ast2::symbol& sym) const noexcept {
                return sym.name() == name;
            }
        };

        struct operator_symbol {
            std::string_view name;
            unsigned precedence;
            bool right_assoc;
        };

        struct prefix_operator_symbol {
            std::string_view name;
        };

        struct symbol_resolution {
            parser_symbol& symbol;
            std::ptrdiff_t distance;
            bool from_parent_scope;
        };

        std::optional<symbol_resolution>
        find_scoped_symbol(const ast2::symbol& sym) {
            const auto it = std::find(_scope_symbols.rbegin(), _scope_symbols.rend(), sym);
            if (it == _scope_symbols.rend()) return std::nullopt;

            const auto current_scope = _scope_symbol_size.back();
            const auto iter_difference = std::distance(_scope_symbols.rbegin(), it);
            return symbol_resolution{
                .symbol = *it,
                .distance = iter_difference,
                .from_parent_scope = current_scope < iter_difference
            };
        }

        bool
        next_relevant();

        void
        enter_scope();

        void
        leave_scope();

        template<class T>
        std::expected<T, source_diagnostic>
        expect_token() {
            if (!_current) // todo make this make sense
                return std::unexpected(source_diagnostic::error(
                    fmt::format(R"(expected "{}" but found end-of-input)", T::token_name),
                    "<unknown>",
                    position{}
                ));
            return std::visit(aux::token_selector<T>{}, *_current);
        }

        prefix_operator_symbol&
        ensure_valid_prefix_operator(const tokens::operator_& sym);

        operator_symbol&
        ensure_valid_operator(const tokens::operator_& sym);

        operator_symbol*
        find_operator(std::string_view name);

        prefix_operator_symbol*
        find_prefix_operator(std::string_view name);

        std::vector<unsigned> _scope_symbol_size;
        std::vector<parser_symbol> _scope_symbols;
        std::vector<unsigned> _scope_operator_size;
        std::vector<operator_symbol> _scope_operators;
        std::vector<unsigned> _scope_prefix_operator_size;
        std::vector<prefix_operator_symbol> _scope_prefix_operators;

        bool _valid{true};
        std::optional<tokens::token_type> _current{};
        lexer _lexer;
    };
}

#endif
