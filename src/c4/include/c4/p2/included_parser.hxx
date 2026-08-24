/* blAST project
 *
 * Copyright (c) 2026 András Bodor <bodand@pm.me>
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
 * Originally created: 2026-08-23.
 *
 * src/c4/include/c4/p2/included_parser --
 *   A special stripped parser that only parses files 1) on the top level and
 *   doesn't produce anything but a list of let declarations that are internal
 *   or public from the parsed file.
 *
 *   Note, that even then, a full-parsing engine is needed because otherwise
 *   weird stuff like ~let fuck_you; could get missed.
 */
#ifndef BLAST_INCLUDED_PARSER_HXX
#define BLAST_INCLUDED_PARSER_HXX

#include <c4/p2/parser-aux.hxx>
#include <c4/p2/parser.hxx>

namespace c4::p2 {
	struct included_parser {
		constexpr static size_t cfg_max_precedence = parser::cfg_max_precedence;

		explicit
		included_parser(ast2::ast_context& context,
		                diagnostics_engine& diag,
		                lexer&& lexer);

		std::vector<ast2::let_expression*>
		parser_global_let();

	private:
		ast2::symbol
		parse_symbol(bool advance = true);

		ast2::symbol
		parse_op_symbol();

		void
		parse_use_expression() {
			/// TODO: implement recursive includes
		}

		void
		throw_away_expression(bool top_level);

		void
		parse_fn_let(enum ast2::let_expression::visibility vis,
		             bool native);

		bool
		parse_associativity_indicator(std::string_view op);

		unsigned
		parse_precedence(std::string_view op);

		void
		parse_op_let(enum ast2::let_expression::visibility vis,
		             bool native);

		void
		parse_let_expression();

		void
		parse_final_expression();

		void
		parse_operator_precedence(unsigned precedence);

		void
		parse_expression();

		void
		next_relevant();

		template<class S, class F>
		bool
		burn_until_paired() {
			if (!expect_token<S>()) return false;
			int depth = 1;
			do {
				next_relevant();
				if (expect_token<S>()) ++depth;
				if (expect_token<F>()) --depth;
			} while (depth > 0);
			return true;
		}

		template<class T>
		std::expected<T, aux::token_error>
		expect_token() {
			return std::visit(aux::token_selector<T>{}, _current);
		}

		[[nodiscard]] position
		current_position() const {
			return std::visit(std::mem_fn(&tokens::token_base::token_position),
			                  _current);
		}

		symbol_table _st{};
		std::vector<ast2::let_expression*> _expressions{};

		diagnostics_engine& _diag;
		ast2::ast_context& _context;
		lexer _lexer;
		tokens::token_type _current;
	};
}

#endif
