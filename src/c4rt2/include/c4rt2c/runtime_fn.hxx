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
 * Originally created: 2026-07-17.
 *
 * src/c4rt2/include/c4rt2c/runtime_fbű --
 *   
 */
#ifndef BLAST_RUNTIME_FBŰ_HXX
#define BLAST_RUNTIME_FBŰ_HXX

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>

namespace c4rt2c {
	struct runtime_fn {
		llvm::FunctionType* type;
		llvm::Function* fn;

		llvm::BasicBlock*
		define(llvm::LLVMContext& ctx) const;

		template<class Fn>
		void
		define(llvm::LLVMContext& ctx, llvm::IRBuilder<>& builder, Fn&& body_builder) const {
			const auto ip = builder.saveIP();
			builder.SetInsertPoint(define(ctx));

			std::vector<llvm::Argument*> args(fn->arg_size());
			std::transform(fn->arg_begin(), fn->arg_end(), args.begin(),
			               [](llvm::Argument& arg) { return &arg; });

			if constexpr (std::convertible_to<decltype(std::invoke(std::forward<Fn>(body_builder), args)),
			                                  llvm::Value*>) {
				const auto ret = std::invoke(std::forward<Fn>(body_builder), args);
				builder.CreateRet(ret);
			}
			else {
				std::invoke(std::forward<Fn>(body_builder), args);
				builder.CreateRetVoid();
			}

			builder.restoreIP(ip);
		}

		explicit(false) operator llvm::FunctionCallee() const noexcept {
			return {type, fn};
		}
	};
}

#endif
