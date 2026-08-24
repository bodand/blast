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
 * src/c4/src/p2/included_parser/throw_away_expression --
 *   
 */

#include <c4/p2/included_parser.hxx>

#include "../parser_utils.hxx"

void
c4::p2::included_parser::
throw_away_expression(const bool top_level) {
	if (burn_until_paired<tokens::lbrace, tokens::rbrace>()) {
		next_relevant();
		return;
	}
	if (burn_until_paired<tokens::lparen, tokens::rparen>()) {
		next_relevant();
		return;
	}

	if (expect_token<tokens::backslash>()) {
		next_relevant();
		_st.enter_scope();
		if (const auto pipe = expect_token<tokens::pipe>()) {
			next_relevant();

			auto sym = expect_token<tokens::bare_symbol>();
			while (sym) {
				_st.declare(sym->name(), 0);
				next_relevant();
				sym = expect_token<tokens::bare_symbol>();
			}

			if (const auto tail = expect_token<tokens::pipe>();
				!tail)
				report_failure(_diag, tail);
			next_relevant();
		}
		throw_away_expression(false);
		_st.leave_scope();
		return;
	}

	unsigned arity = 0;
	if (burn_until_paired<tokens::ampersand, tokens::arity_marker>()) {
		arity = expect_token<tokens::arity_marker>()->arity();
		next_relevant();
	}
	else if (expect_token<tokens::bare_symbol>()) {
		auto sym = parse_symbol();
		const auto resolved = _st.find_scoped_symbol(sym);
		if (!resolved) {
			/// XXX add flaky recover logic, maybe we can produce something
			/// potentially helpful
			_diag.error(sym.position(),
			            "unknown symbol referenced in function call: {}",
			            sym.name());
			throw bad_token_error{};
		}
		sym = sym.with_arity(resolved->symbol.arity);
		sym.references(resolved->symbol.referee);

		arity = sym.base_arity();
	}
	else if (const auto pfx = expect_token<tokens::operator_>()) {
		next_relevant();

		const auto op_sym = ast2::symbol(pfx->token_position(),
		                                 pfx->value(),
		                                 1);
		if (!_st.find_scoped_symbol_with_arity(op_sym)) {
			_diag.error(pfx->token_position(),
			            "unknown prefix operator referenced: {}/1",
			            pfx->value())
			     .when(_st.find_infix_operator(pfx->value()))
			     .note("there exists an infix operator with name {}/2, did you mean to call that?",
			           pfx->value());
		}

		arity = 1;
	}

	for (unsigned i = 0; i < arity; ++i) {
		if (top_level) {
			parse_expression();
		}
		else {
			throw_away_expression(false);
		}
	}
}
