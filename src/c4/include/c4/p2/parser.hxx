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

#ifndef __clang__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wchanges-meaning"
#endif

#include <expected>
#include <list>
#include <stdexcept>
#include <utility>

#include <c4/ast2/ast_context.hxx>
#include <c4/ast2/expression.hxx>
#include <c4/ast2/float_literal.hxx>
#include <c4/ast2/integer_literal.hxx>
#include <c4/ast2/string_literal.hxx>
#include <c4/ast2/symbol.hxx>

#include <c4/p2/lex/lexer.hxx>

namespace c4::p2 {
	struct bad_token_error final : std::runtime_error {
		bad_token_error()
			: std::runtime_error("parser failure: bad token") { }
	};

	namespace aux {
		struct mismatched_token_error final {
			std::string_view name;
			std::string_view expected;
			position position;
		};

		struct eof_error final {
			constexpr static std::string_view name = "end of file";
			std::string_view expected;
			position position;
		};

		template<class>
		struct type_val_t { };

		template<class T>
		constexpr static auto type_val = type_val_t<T>{};

		struct token_error final {
			using value_type = std::variant<mismatched_token_error, eof_error>;

			template<class T, class... Args>
			explicit(false)
			token_error(type_val_t<T>, Args&&... args)
				: _value{std::in_place_type<T>, std::forward<Args>(args)...} { }

			explicit
			token_error(value_type value)
				: _value{std::move(value)} { }

			[[nodiscard]] std::string_view
			name() const {
				return std::visit([](const auto& tok) { return tok.name; }, _value);
			}

			[[nodiscard]] std::string_view
			expected_token_name() const {
				return std::visit([](const auto& tok) { return tok.expected; }, _value);
			}

			[[nodiscard]] position
			position() const {
				return std::visit([](const auto& tok) { return tok.position; }, _value);
			}

		private:
			value_type _value;
		};

		template<class T>
		struct token_selector {
			std::expected<T, token_error>
			operator()(const T& tok) const { return tok; }

			template<class Found>
			std::expected<T, token_error>
			operator()(const Found& tok) const {
				return std::unexpected<token_error>(
					std::in_place,
					type_val<mismatched_token_error>,
					Found::token_name,
					T::token_name,
					tok.token_position());
			}
		};
	}

	struct parser {
		explicit
		parser(ast2::ast_context& context,
		       diagnostics_engine& diagnostics_engine,
		       lexer&& lexer);

		ast2::integer_literal
		parse_integer_literal();

		ast2::float_literal
		parse_float_literal();

		ast2::string_literal
		parse_string_literal();

		ast2::symbol
		parse_symbol();

		ast2::symbol
		parse_op_symbol();

		ast2::symbol
		parse_bare_symbol();

		ast2::expression*
		parse_expression();

		ast2::expression*
		parse_let_expression();

		ast2::expression*
		parse_final_expression();

		ast2::block*
		parse_block();

		ast2::block_args*
		parse_block_args();

		std::vector<ast2::expression*>
		parse_script();

		void
		declare_symbol(std::string_view symbol, unsigned arity, ast2::tags::referable* referee);

		void
		declare_binop(std::string_view symbol, unsigned precedence, bool right_assoc);

		void
		declare_uniop(std::string_view symbol);

		[[nodiscard]] std::vector<ast2::undef_symbol>
		promised_symbols() const;

	private:
		bool
		parse_associativity_indicator(std::string_view op);

		unsigned
		parse_precedence(std::string_view op);

		void
		parse_n_expressions(unsigned n,
		                    std::vector<ast2::expression*>& expressions);

		ast2::expression*
		parse_operator_precedence(ast2::expression* lhs, unsigned precedence);

		struct parser_symbol {
			std::string_view name;
			ast2::tags::referable* referee;
			unsigned arity;
			unsigned precedence; // Set only on operators
			bool right_assoc;    // Set only on operators

			parser_symbol(const std::string_view& name_,
			              const unsigned arity_,
			              ast2::tags::referable* referee_,
			              const unsigned precedence_ = 0,
			              const bool right_assoc_ = false)
				noexcept(std::is_nothrow_copy_constructible_v<std::string_view>)
				: name{name_}
				, referee{referee_}
				, arity{arity_}
				, precedence{precedence_}
				, right_assoc{right_assoc_} { }

			[[nodiscard]] bool
			is_operator() const noexcept { return precedence != 0; }

			[[nodiscard]] bool
			operator==(const ast2::symbol& sym) const noexcept {
				return sym.name() == name;
			}
		};

		struct symbol_resolution {
			parser_symbol& symbol;
			std::ptrdiff_t distance;
			bool save_in_context;
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
				.save_in_context = std::cmp_greater_equal(iter_difference, current_scope)
			};
		}

		std::optional<symbol_resolution>
		find_scoped_symbol_with_arity(const ast2::symbol& sym) {
			const auto it = std::find_if(_scope_symbols.rbegin(), _scope_symbols.rend(), [&sym](const auto& scope) {
				return scope.name == sym.name() && scope.arity == sym.base_arity();
			});
			if (it == _scope_symbols.rend()) return std::nullopt;

			const auto current_scope = _scope_symbol_size.back();
			const auto iter_difference = std::distance(_scope_symbols.rbegin(), it);
			return symbol_resolution{
				.symbol = *it,
				.distance = iter_difference,
				.save_in_context = std::cmp_greater(iter_difference, current_scope)
			};
		}

		void
		next_relevant();

		void
		enter_scope();

		void
		leave_scope();

		parser_symbol&
		declare_symbol_internal(std::string_view symbol,
		                        unsigned arity,
		                        ast2::tags::referable* referee = nullptr,
		                        unsigned precedence = 0,
		                        bool right_assoc = false);

		template<class T>
		std::expected<T, aux::token_error>
		expect_token() {
			return std::visit(aux::token_selector<T>{}, _current);
		}

		parser_symbol&
		ensure_valid_prefix_operator(const tokens::operator_& sym);

		parser_symbol&
		ensure_valid_infix_operator(const tokens::operator_& sym);

		parser_symbol*
		find_infix_operator(std::string_view name);

		parser_symbol*
		find_prefix_operator(std::string_view name);

		std::vector<unsigned> _scope_symbol_size;
		std::vector<parser_symbol> _scope_symbols;

		// unsigned _dynamic_call_index{};
		// std::list<std::string> _dynamic_call_buffers;

		diagnostics_engine& _diag;
		ast2::ast_context& _context;
		lexer _lexer;
		tokens::token_type _current;
	};
}

#ifndef __clang__
#pragma GCC diagnostic pop
#endif

#endif
