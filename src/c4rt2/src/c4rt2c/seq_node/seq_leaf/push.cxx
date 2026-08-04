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
 * Originally created: 2026-07-30.
 *
 * src/c4rt2/src/c4rt2c/seq_node/seq_leaf/push --
 *
 */

#include <c4/ast2/expression.hxx>

#include <c4rt2c/ast2_ir_emitter.hxx>
#include <c4rt2c/seq_node.hxx>

#include <libassert/assert.hpp>

std::unique_ptr<c4rt2c::seq_node>
c4rt2c::seq_leaf::
push(const c4::ast2::expression* val, ast2_ir_emitter& ir) {
	val->tail_call(false);
	val->accept(ir);
	return push(val);
}

std::unique_ptr<c4rt2c::seq_node>
c4rt2c::seq_leaf::
push(const c4::ast2::expression* val) {
	const auto value = val->attribute_value<llvm::Value*>("value");
	ASSERT(value, "expression not defined to value", val);
	const auto thunk = val->attribute_value<bool>("thunk?");
	ASSERT(thunk, "non-thunked expr cannot be pushed to leaf", val);

	return std::make_unique<seq_pair>(
		std::make_unique<seq_leaf>(_value),
		std::make_unique<seq_leaf>(*value)
	);
}
