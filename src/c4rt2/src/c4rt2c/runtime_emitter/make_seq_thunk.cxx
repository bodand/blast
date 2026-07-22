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
 * Originally created: 2026-07-22.
 *
 * src/c4rt2/src/c4rt2c/runtime_emitter/make_seq_thunkú --
 *   
 */

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Value.h>

#include <c4rt2c/runtime_emitter.hxx>

llvm::Value*
c4rt2c::runtime_emitter::make_seq_thunk(llvm::Value* thunk_left, llvm::Value* thunk_right) const {
	const auto ptr_t = llvm::PointerType::get(_context, 0);

	const auto seq = make_thunk(_rt_seq.fn);
	seq->setName("seq.thunk");

	const auto argv = allocate_array(2, 8);
	argv->setName("seq.argv");

	const auto addr0 = _builder.CreateGEP(ptr_t, argv, {llvm::ConstantInt::get(_context, llvm::APInt(64, 0))});
	_builder.CreateStore(thunk_left, addr0);

	const auto addr1 = _builder.CreateGEP(ptr_t, argv, {llvm::ConstantInt::get(_context, llvm::APInt(64, 1))});
	_builder.CreateStore(thunk_right, addr1);

	set_thunk_args(seq, argv);

	return seq;
}
