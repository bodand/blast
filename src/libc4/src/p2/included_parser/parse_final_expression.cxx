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
 * Originally created: 2026-08-24.
 *
 * src/c4/src/p2/included_parser/parse_final_expression --
 *   
 */

#include <c4/p2/included_parser.hxx>

#include "../parser_utils.hxx"

void
c4::p2::included_parser::
parse_final_expression() {
	const auto lpar = expect_token<tokens::lparen>();
	if (lpar) {
		next_relevant(); // (
		parse_expression();
		// )
		if (const auto rpar = expect_token<tokens::rparen>();
			!rpar)
			report_failure(_diag, rpar);

		next_relevant();
		return;
	}

	const auto str = expect_token<tokens::string_literal>();
	if (str) {
		next_relevant();
		return;
	}

	const auto flt = expect_token<tokens::float_literal>();
	if (flt) {
		next_relevant();
		return;
	}

	const auto integer = expect_token<tokens::integer_literal>();
	if (integer) {
		next_relevant();
		return;
	}

	const auto symbol = expect_token<tokens::symbol>();
	if (symbol) {
		next_relevant();
		return;
	}

	const auto prefix_op = expect_token<tokens::bare_symbol>();
	if (prefix_op) return throw_away_expression(true);

	const auto fn_symbol = expect_token<tokens::bare_symbol>();
	if (fn_symbol) return throw_away_expression(true);

	const auto lbrace = expect_token<tokens::lbrace>();
	if (lbrace) return throw_away_expression(true);

	const auto backslash = expect_token<tokens::backslash>();
	if (backslash) return throw_away_expression(true);

	const auto dyn_call_start = expect_token<tokens::ampersand>();
	if (dyn_call_start) return throw_away_expression(true);

	report_failure(_diag, lpar, str, flt, integer, symbol, prefix_op, lbrace, backslash, fn_symbol, dyn_call_start);
}
