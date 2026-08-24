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
 * src/c4/src/p2/included_parser/parse_let_expression --
 *   
 */

#include <c4/p2/included_parser.hxx>

#include "../parser_utils.hxx"

void
c4::p2::included_parser::
parse_let_expression() {
	if (const auto let = expect_token<tokens::let>();
		!let)
		report_failure(_diag, let);
	next_relevant();

	auto vis = ast2::let_expression::v_internal;
	if (const auto vis_tok = expect_token<tokens::operator_>()) {
		if (vis_tok->value() == "+") vis = ast2::let_expression::v_public;
		else if (vis_tok->value() == "-") vis = ast2::let_expression::v_private;
		else if (vis_tok->value() == "~") vis = ast2::let_expression::v_internal;
		else report_failure(_diag, vis_tok);

		next_relevant();
	}

	bool native = false;
	if (const auto bare_symbol = expect_token<tokens::bare_symbol>()) {
		if (bare_symbol->name() == "native") {
			native = true;
			next_relevant();
		}
	}

	if (expect_token<tokens::operator_symbol>()
	    || expect_token<tokens::fn_operator>())
		return parse_op_let(vis, native);

	return parse_fn_let(vis, native);
}
