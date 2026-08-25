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
 * src/c4rt2/src/c4rt2c/runtime_emitter/allocate_array --
 *   
 */

#include <c4rt2c/runtime_emitter.hxx>

#include <llvm/IR/Constants.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Value.h>

llvm::Value*
c4rt2c::runtime_emitter::
allocate_array(llvm::Value* count, llvm::Value* size, const std::string_view debug_loc) const {
	llvm::CallInst* call = nullptr;
	if (debug.debug_gc) {
		const auto str_loc = get_debug_location(debug_loc);
		call = _builder.CreateCall(_rt_allocate_array_debug, {count, size, str_loc});
	}
	else {
		call = _builder.CreateCall(_rt_allocate_array, {count, size});
	}

	call->setCallingConv(llvm::CallingConv::Tail);
	return call;
}

llvm::Value*
c4rt2c::runtime_emitter::
allocate_array(const std::size_t count, const std::size_t size, const std::string_view debug_loc) const {
	// if (count == 0) return llvm::ConstantPointerNull::get(ptr_t);

	return allocate_array(
		llvm::ConstantInt::get(_context, llvm::APInt(64, count)),
		llvm::ConstantInt::get(_context, llvm::APInt(64, size)),
		debug_loc);
}
