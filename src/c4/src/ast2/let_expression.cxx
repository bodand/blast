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
 * src/c4/src/ast2/let_expression --
 *   
 */

#include <c4/ast2/expression.hxx>
#include <c4/ast2/let_expression.hxx>

#include <utility>
#include <libassert/assert.hpp>

c4::ast2::let_expression::let_expression(const c4::position& position,
                                         ast2::symbol symbol,
                                         struct expression* expr)
	: referable{position}
	, _symbol{std::move(symbol)}
	, _value{expr} {
	_symbol.references(this);
	mark_expression_owned();
}

void
c4::ast2::let_expression::mark_expression_owned() {
	if (!_value) return;
	_value->owner(this);
}

void
c4::ast2::let_expression::expression(ast2::expression* expr) {
	ASSERT(_value == nullptr, "let-expression's value is already set", _symbol);
	_value = expr;
	mark_expression_owned();
}

const c4::ast2::expression&
c4::ast2::let_expression::value() const {
	DEBUG_ASSERT(_value != nullptr, "let-expression's value is not set", _symbol);
	return *_value;
}

bool
c4::ast2::let_expression::pseudo_let() const noexcept {
	const auto pseudo_let = attribute_value<bool>("pseudo_let");
	if (!pseudo_let) return false;
	return *pseudo_let;
}

bool
c4::ast2::let_expression::introduces_variable() const noexcept {
	if (_value->true_closure()) return false;
	if (_value->invocable_with()) return false;
	return true;
}

bool
c4::ast2::let_expression::global_symbol() const {
	const auto in = attribute_value<block*>("nested-in");
	return !in.has_value();
}

c4::ast2::block*
c4::ast2::let_expression::function_body() const noexcept {
	ASSERT(introduces_function(), "let is not function");

	return std::get<block*>(_value->value());
}
