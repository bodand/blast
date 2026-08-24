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
 * src/c4/src/p2/included_parser/larse_op_len --
 *   
 */

#include <c4/p2/included_parser.hxx>

#include <c4/ast2/ast_context.hxx>

void
c4::p2::included_parser::
parse_op_let(const enum ast2::let_expression::visibility vis,
             const bool native) {
	auto op = parse_op_symbol();
	const auto let = _context.build_let_expression(
		op.position(),
		op,
		nullptr,
		vis
	);

	if (native) {
		_diag.error(op.position(),
		            "operator `{}/{}' cannot be declared native",
		            op.name(), op.base_arity())
		     .note("native modifier is ignored")
		     .note("consider defining a private function and wraping it",
		           op.name());
	}

	if (const auto last = _st.find_scoped_symbol_with_arity(op)) {
		_diag.error(op.position(),
		            "operator `({})/{}' is already defined",
		            op.name(), op.base_arity())
		     .note("replacing definition with this one for further parsing")
		     .when(last->symbol.referee)
		     .note(last->symbol.referee->position(), "previous definition is here")
		     .when(!last->symbol.referee)
		     .note("previous definition was externally provided");
	}

	switch (op.base_arity()) {
	case 0:
		_diag.error(op.position(),
		            "invalid operator arity: {} for operator `({})/0' (expected 1 or 2)",
		            op.base_arity(),
		            op.name())
		     .note("reparsing as if it had one argument (prefix)");
		op = op.with_arity(1);
		[[fallthrough]];
	case 1:
		_st.declare(op.name(),
		            op.base_arity(),
		            let,
		            static_cast<unsigned>(-1));
		break;

	default:
		_diag.error(op.position(),
		            "invalid operator arity: {} for operator `({})/?' (expected 1 or 2)",
		            op.base_arity(),
		            op.name())
		     .note("reparsing as if it had two arguments (infix)");
		op = op.with_arity(2);
		[[fallthrough]];
	case 2:
		const auto left_assoc = parse_associativity_indicator(op.name());
		next_relevant();
		const unsigned precedence = parse_precedence(op.name());
		next_relevant();
		op.operator_data(left_assoc, precedence);

		_st.declare(op.name(),
		            op.base_arity(),
		            let,
		            precedence,
		            !left_assoc);
		break;
	}

	throw_away_expression(false);

	let->symbol(op);
	_expressions.push_back(let);
}
