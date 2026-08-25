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
 * src/c4/src/p2/included_parser/parse_fn_let --
 *   
 */

#include <c4/p2/included_parser.hxx>

#include <c4/ast2/ast_context.hxx>

#include "../parser_utils.hxx"

void
c4::p2::included_parser::
parse_fn_let(const enum ast2::let_expression::visibility vis,
             const bool native) {
	const auto symbol = parse_symbol(false);

	ast2::let_expression* let = nullptr;

	if (const auto last = _st.find_scoped_symbol(symbol)) {
		if (last->symbol.referee) {
			let = dynamic_cast<ast2::let_expression*>(last->symbol.referee);
			if (let) {
				if (let->symbol().base_arity() != symbol.base_arity()) {
					_diag.error(symbol.position(),
					            "function `{}' is already defined with different arity",
					            symbol.name())
					     .note(last->symbol.referee->position(), "previous definition is here with arity {}",
					           let->symbol().base_arity())
					     .note("using previous definition for further parsing");
				}
				else if (!let->declaration()) {
					_diag.error(symbol.position(),
					            "function `{}' is already defined",
					            symbol.name())
					     .note("replacing definition with this one for further parsing")
					     .note(last->symbol.referee->position(), "previous definition is here");
				}
			}
			else {
				_diag.warning(symbol.position(),
				              "local variable shadows argument of enclosing block");
			}
		}
	}

	if (!let) {
		let = _context.build_let_expression(
			symbol.position(),
			symbol,
			nullptr,
			vis
		);
		_st.declare(symbol.name(), symbol.base_arity(), let);
	}

	if (native) {
		if (!expect_token<tokens::symbol>()) {
			_diag.warning(current_position(),
			              "native function `{}/{}' is declared with bare symbol (without arity)",
			              symbol.name(),
			              symbol.base_arity())
			     .note("consider using {}/0", symbol.name());
		}

		let->emplace_attribute<native_attachment>("native",
		                                          symbol.with_native());
		next_relevant();

		if (!expect_token<tokens::semicolon>()) {
			_diag.error(let->position(),
			            "function `{}/{}' is marked as `native' but given definition",
			            symbol.name(),
			            symbol.base_arity())
			     .note(current_position(),
			           "expected `;' to follow declaration");
		}
		else next_relevant(); // skip semicolon

		std::vector symbol_stack{symbol};
		let->emplace_attribute<namespaced_symbol_attribute>("symbol-stack", std::move(symbol_stack));

		_expressions.push_back(let);
		return;
	}
	next_relevant(); // advance manually because we did not ask parse_symbol to

	throw_away_expression(false);

	_expressions.push_back(let);
}
