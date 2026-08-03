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
 * Originally created: 2026-08-03.
 *
 * src/c4rt2/src/c4rt2c/fn_call_gen/tail_fn_call_gen/finalize --
 *   
 */

#include <c4/ast2/expression.hxx>

#include <c4rt2c/fn_call_gen.hxx>
#include <c4rt2c/llvm_value_attribute.hxx>

namespace {
	struct callee_pair_attribute : c4::ast2::tags::typed_attribute<std::pair<llvm::Value*, llvm::Value*>> {
		explicit
		callee_pair_attribute(llvm::Value* argv, llvm::Value* argv_sz)
			: typed_attribute{std::make_pair(argv, argv_sz)} { }
	};
}

void
c4rt2c::tail_fn_call_gen::
finalize(llvm::Value* fn,
         llvm::Value* argv,
         const std::size_t argv_sz,
         const c4::ast2::expression& expr) {
	const auto argv_sz_val = _builder.getInt32(argv_sz);

	expr.emplace_attribute<c4c::llvm_value_attribute>("value", fn);
	expr.emplace_attribute<callee_pair_attribute>("tail_args", argv, argv_sz_val);
}
