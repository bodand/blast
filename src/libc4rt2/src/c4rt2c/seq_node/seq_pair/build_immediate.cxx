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
 * Originally created: 2026-08-05.
 *
 * src/c4rt2/src/c4rt2c/seq_node/seq_pair/build_immediate --
 *   
 */

#include <c4rt2c/runtime_emitter.hxx>
#include <c4rt2c/seq_node.hxx>

std::optional<c4rt2c::immediate_seq>
c4rt2c::seq_pair::build_immediate(llvm::IRBuilder<>& builder,
                                  runtime_emitter& rt) const {
	const auto left_thunk = _left->build(builder, rt);
	if (const auto imm = _right->build_immediate(builder, rt)) {
		const auto argv = rt.make_seq_ti_argv(left_thunk, imm->fn, imm->argv);
		return immediate_seq{.fn = rt.seq_ti(), .argv = argv};
	}

	const auto right_thunk = _right->build(builder, rt);
	const auto argv = rt.make_seq_tt_argv(left_thunk, right_thunk);
	return immediate_seq{.fn = rt.seq_tt(), .argv = argv};
}
