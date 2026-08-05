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
 * src/c4rt2/src/c4rt2c/runtime_emitter/make_seq_ti_argv --
 *   
 */

#include <c4rt2c/runtime_emitter.hxx>

llvm::Value*
c4rt2c::runtime_emitter::
make_seq_ti_argv(llvm::Value* left_thunk,
                 llvm::Function* fn,
                 llvm::Value* fn_argv) const {
	const auto layout = _module.getDataLayout();

	const auto alignment = llvm::Align(layout.getPointerABIAlignment(0));
	const auto argv = with_name(allocate_array(3, layout.getPointerSize(0)), "seq_ti.argv");

	const auto left_addr = _builder.CreateGEP(ptr_t, argv, _builder.getInt64(0));
	_builder.CreateAlignedStore(left_thunk, left_addr, alignment);

	const auto fn_addr = _builder.CreateGEP(ptr_t, argv, _builder.getInt64(1));
	_builder.CreateAlignedStore(fn, fn_addr, alignment);

	const auto argv_addr = _builder.CreateGEP(ptr_t, argv, _builder.getInt64(2));
	_builder.CreateAlignedStore(fn_argv, argv_addr, alignment);

	return argv;
}
