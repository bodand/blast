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
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Originally created: 2026-03-03.
 *
 * src/c4/src/ast3_mapper --
 *
 */

#include <c4/ast2/expression.hxx>
#include <c4/ast2/block.hxx>
#include <c4/ast2/let_expression.hxx>
#include <c4/ast2/float_literal.hxx>
#include <c4/ast2/fn_call.hxx>
#include <c4/ast2/integer_literal.hxx>
#include <c4/ast2/string_literal.hxx>
#include <c4/ast2/op_call.hxx>
#include <c4/ast2/dynamic_call.hxx>
#include <c4/ast2/symbol.hxx>

#include <c4/ast3/primitive.hxx>
#include <c4/ast3_mapper.hxx>

#include <libassert/assert.hpp>

#include <fmt/core.h>

void
c4::ast3_mapper::do_visit(const ast2::float_literal& obj) {
	_context.build_primitive(obj.value());
}

void
c4::ast3_mapper::do_visit(const ast2::integer_literal& obj) {
	_context.build_primitive(obj.value());
}

void
c4::ast3_mapper::do_visit(const ast2::string_literal& obj) {
	_context.build_primitive(obj.value());
}

void
c4::ast3_mapper::do_visit(const ast2::fn_call& obj) { }

void
c4::ast3_mapper::do_visit(const ast2::binary_op_call& obj) { }

void
c4::ast3_mapper::do_visit(const ast2::unary_op_call& obj) { }

void
c4::ast3_mapper::do_visit(const ast2::dynamic_call& obj) { }

void
c4::ast3_mapper::do_visit(const ast2::let_expression& obj) {
	enter_function_definition({obj.name().data(), obj.name().size()});
	obj.value().accept(*this);
	leave_function_definition();
}

void
c4::ast3_mapper::do_visit(const ast2::expression& obj) {
	obj.accept_skip_self(*this);
}

void
c4::ast3_mapper::define_function(ast2::block& obj) {
	ASSERT(last_function()->taken(), "untaken function cannot be defined");
	const auto& expressions = obj.expressions();
	std::for_each(expressions.rbegin(), expressions.rend(), [&](const auto& expr) {
		expr->accept(*this);
	});
}

void
c4::ast3_mapper::enter_function_definition(const std::string& fn) {
	ASSERT(!fn.empty(), "entering invalid function: empty name", _function_stack);
	_function_stack.emplace_back(_function_stack, fn);
}

void
c4::ast3_mapper::leave_function_definition() {
	ASSERT(
		!_function_stack.empty(),
		"trying to leave from non-existent function definition"
	);
	_function_stack.pop_back();
}

c4::ast3_mapper::function_stack_element*
c4::ast3_mapper::last_function() {
	ASSERT(
		!_function_stack.empty(),
		"trying to reference non-existent function definition"
	);
	return &_function_stack.back();
}

void
c4::ast3_mapper::do_visit(const ast2::block& obj) {
	if (last_function()->take()) {
		// we are in a let-named function block, build function body as is
		// expected with the name
		/// ...
	}
	else {
		// we are an anonymous block -> generate name based on parent
		const auto named_anon = last_function()->next_anonymous();
		enter_function_definition(named_anon);
		ASSERT(last_function()->take(), "anon function miraculously taken", named_anon);
		/// ...
		leave_function_definition();
	}
}

std::string
c4::ast3_mapper::function_stack_element::next_anonymous() {
	ASSERT(
		_anonymous_counter != std::numeric_limits<decltype(_anonymous_counter)>::max(),
		"anonymous function counter overflowed: I'm sorry, what?"
	);

	return fmt::format("__anon{}", _anonymous_counter++);
}
