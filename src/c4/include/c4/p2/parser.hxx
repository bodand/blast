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
#include <deque>

#include "../../../src/p2/symbol_table.hxx"

#ifndef __clang__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wchanges-meaning"
#endif

#include <expected>
#include <list>
#include <utility>

#include <c4/ast2/ast_context.hxx>
#include <c4/ast2/expression.hxx>
#include <c4/ast2/float_literal.hxx>
#include <c4/ast2/integer_literal.hxx>
#include <c4/ast2/string_literal.hxx>
#include <c4/ast2/symbol.hxx>

#include <c4/p2/lex/lexer.hxx>

#include <c4/p2/parser-aux.hxx>

namespace c4::p2 {
	struct parser {
		constexpr static size_t cfg_max_precedence = 100;

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
		parse_symbol(bool advance = true);

		ast2::symbol
		parse_op_symbol();

		ast2::symbol
		parse_bare_symbol();

		ast2::expression*
		parse_use_expression();

		ast2::expression*
		parse_expression();

		ast2::expression*
		parse_operator_let(enum ast2::let_expression::visibility vis, bool native);

		ast2::expression*
		parse_fn_let(enum ast2::let_expression::visibility vis, bool native);

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

	private:
		static void
		set_symbol_stack(const ast2::symbol& symbol,
							  const ast2::let_expression* let,
							  const ast2::let_expression* memory);

		ast2::expression*
		parse_expression_of_let(const ast2::symbol& symbol,
		                        ast2::let_expression* let);

		bool
		parse_associativity_indicator(std::string_view op);

		unsigned
		parse_precedence(std::string_view op);

		void
		parse_n_expressions(unsigned n,
		                    std::vector<ast2::expression*>& expressions);

		ast2::expression*
		parse_operator_precedence(ast2::expression* lhs, unsigned precedence);

		[[nodiscard]] position
		current_position() const {
			return std::visit(std::mem_fn(&tokens::token_base::token_position),
			                  _current);
		}

		void
		next_relevant();

		template<class T>
		std::expected<T, aux::token_error>
		expect_token() {
			return std::visit(aux::token_selector<T>{}, _current);
		}

		parser_symbol&
		ensure_valid_prefix_operator(const tokens::operator_& sym);

		parser_symbol&
		ensure_valid_infix_operator(const tokens::operator_& sym);

		symbol_table _st;

		ast2::let_expression* _within_let = nullptr;
		ast2::block* _within_block = nullptr;

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
