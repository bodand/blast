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
 * src/c4rt2/include/c4rt2c/runtime_emitter --
 *   
 */
#ifndef BLAST_RUNTIME_EMITTER_HXX
#define BLAST_RUNTIME_EMITTER_HXX

#include <c4rt2c/runtime_fn.hxx>

#include <llvm/IR/IRBuilder.h>

namespace llvm {
	class FunctionType;
	class LLVMContext;
	class Module;
	class Value;
}

namespace c4rt2c {
	struct runtime_emitter {
		runtime_emitter(llvm::LLVMContext& ctx, llvm::Module& module, llvm::IRBuilder<>& builder);

		[[nodiscard]] llvm::FunctionType*
		function_type() const { return _function_type; }

		llvm::Value*
		make_thunk(llvm::Value* fnptr);

		void
		set_thunk_args(llvm::Value* thunk, llvm::Value* argv);

		[[nodiscard]] llvm::Value*
		make_datum_str(std::string_view str, const std::optional<std::string_view>& global_name = {}) const;

	private:
		llvm::LLVMContext& _context;
		llvm::Module& _module;
		llvm::IRBuilder<>& _builder;

		/// The universal function type to allow unrestricted
		/// tail-calls. It is void(ptr, ptr), where the first is an array
		/// to pointers as "argv" and the latter is the K continuation.
		llvm::FunctionType* _function_type;

		runtime_fn _rt_make_thunk;
		runtime_fn _rt_set_thunk_args;
		runtime_fn _rt_make_datum_str;
		runtime_fn _rt_make_datum_int64;
		runtime_fn _rt_make_datum_float64;
		runtime_fn _rt_make_datum_block;
		runtime_fn _rt_allocate_array;
		runtime_fn _rt_evaluate;
		runtime_fn _rt_seq;
		runtime_fn _rt_complete_thunk;
	};
}

#endif
