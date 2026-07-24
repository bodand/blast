/* blAST project
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
 * src/c4/include/c4/ast2/op_call --
 *   
 */

#include <c4/ast2/op_call.hxx>
#include <c4/ast2/expression.hxx>

#include <libassert/assert.hpp>

c4::ast2::binary_op_call::binary_op_call(const c4::position& position,
                                         const symbol& op,
                                         expression* left,
                                         expression* right)
	: source_positioned{position}
	, _op{op}
	, _args{left, right} {
	DEBUG_ASSERT(
		_op.base_arity() == 2,
		"non-binary operator passed to binary expression",
		_op
	);
	DEBUG_ASSERT(
		left != nullptr,
		"binary operator cannot have null subexpression (left operand)"
	);
	DEBUG_ASSERT(
		right != nullptr,
		"binary operator cannot have null subexpression (right operand)"
	);
}

const c4::ast2::expression&
c4::ast2::binary_op_call::left() const noexcept { return *_args[0]; }

const c4::ast2::expression&
c4::ast2::binary_op_call::right() const noexcept { return *_args[1]; }

bool
c4::ast2::binary_op_call::constant_evaluated() const noexcept {
	// TODO: op symbol upgrade : see fn_call::constant_evaluated
	return ast_node::constant_evaluated()
	       && _args[0]->constant_evaluated()
	       && _args[1]->constant_evaluated();
}

c4::ast2::unary_op_call::unary_op_call(const c4::position& position,
                                       const symbol& op,
                                       expression* operand)
	: source_positioned{position}
	, _op{op}
	, _operand{operand} {
	DEBUG_ASSERT(
		_op.base_arity() == 1,
		"unary operator passed to unary expression",
		_op
	);
	DEBUG_ASSERT(
		_operand != nullptr,
		"unary operator cannot have null subexpression (operand)"
	);
}

const c4::ast2::expression&
c4::ast2::unary_op_call::operand() const noexcept { return *_operand; }

bool
c4::ast2::unary_op_call::constant_evaluated() const noexcept {
	// TODO: op symbol upgrade : see fn_call::constant_evaluated
	return ast_node::constant_evaluated()
	       && _operand->constant_evaluated();
}
