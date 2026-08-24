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
 * src/c4/src/p2/included_parser/parse_operator_precedence --
 *   
 */

#include <c4/p2/included_parser.hxx>
#include <libassert/assert.hpp>

#include "../parser_utils.hxx"

void
c4::p2::included_parser::
parse_operator_precedence(const unsigned precedence) {
	auto lookahead = expect_token<tokens::operator_>();
	if (!lookahead) return;

	auto op_token = *lookahead;
	auto op = _st.find_infix_operator(op_token.value());
	if (!op) report_failure(_diag, lookahead);

	while (op->precedence >= precedence) {
		next_relevant();
		parse_final_expression();
		lookahead = expect_token<tokens::operator_>();
		if (lookahead) {
			auto op_ahead = _st.find_infix_operator(lookahead->value());
			if (!op_ahead) report_failure(_diag, lookahead);

			while (lookahead && (op_ahead->precedence > op->precedence
			                     || (op_ahead->right_assoc && op_ahead->precedence == op->precedence))) {
				parse_operator_precedence(op->precedence + (op_ahead->precedence > op->precedence));
				lookahead = expect_token<tokens::operator_>();
				if (!lookahead) continue;

				op_ahead = _st.find_infix_operator(lookahead->value());
				if (!op_ahead) report_failure(_diag, lookahead);
			}
		}

		ast2::symbol op_sym(op_token.token_position(),
		                    op_token.value(),
		                    2);

		if (!lookahead) break;
		op_token = *lookahead;
		op = _st.find_infix_operator(lookahead->value());
	}
}
