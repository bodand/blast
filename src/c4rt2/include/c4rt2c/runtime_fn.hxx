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
 * src/c4rt2/include/c4rt2c/runtime_fn --
 *
 */
#ifndef BLAST_RUNTIME_FN_HXX
#define BLAST_RUNTIME_FN_HXX

#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>

#include "scoped_scope.hxx"
#include "../../../../vcpkg/buildtrees/llvm/src/org-18.1.6-e754cb1d0b.clean/llvm/include/llvm/IR/DIBuilder.h"

namespace llvm {
	class DIBuilder;
}

namespace c4rt2c {
	struct runtime_fn;

	struct runtime_fn_debug_info_builder {
		runtime_fn_debug_info_builder&
		name(const std::string_view name) {
			_name = name;
			return *this;
		}

		runtime_fn_debug_info_builder&
		file_scoped(llvm::DIFile* file);

		runtime_fn_debug_info_builder&
		returns(llvm::DIType* ret) {
			_ret_type = ret;
			return *this;
		}

		runtime_fn_debug_info_builder&
		arg(std::string_view name,
		    llvm::DIType* type,
		    llvm::DIExpression* expr = nullptr);

		~runtime_fn_debug_info_builder();

	private:
		struct arg {
			std::string_view name;
			llvm::DIType* type;
			llvm::DIExpression* expr;
		};

		runtime_fn_debug_info_builder(runtime_fn* target,
		                              llvm::DIBuilder* dib)
			: _target{target}
			, _dib{dib} { }

		std::string_view _name{};
		llvm::DIFile* _file{};
		llvm::DIScope* _scope{};
		unsigned _line{0};
		unsigned _scope_line{0};
		llvm::DIType* _ret_type{};
		llvm::SmallVector<struct arg, 4> _args{};

		friend runtime_fn;
		runtime_fn* _target;
		llvm::DIBuilder* _dib;
	};

	struct runtime_fn {
		llvm::FunctionType* type;
		llvm::Function* fn;

		mutable llvm::DIBuilder* _dib{};
		mutable llvm::BasicBlock* _entry{};

		runtime_fn_debug_info_builder
		dbg(llvm::DIBuilder* dib) { return {this, dib}; }

		llvm::BasicBlock*
		define(llvm::LLVMContext& ctx) const;

		template<class Fn>
		void
		define(llvm::IRBuilder<>& builder, Fn&& body_builder) const {
			const scoped_scope scope(builder);
			builder.SetInsertPoint(entry());

			const auto dbg = fn->getSubprogram();
			if (dbg) {
				auto& ctx = builder.getContext();
				scope.push_dbg(llvm::DILocation::get(ctx, 0, 0, dbg));
			}

			llvm::SmallVector<llvm::Argument*, 4> args(fn->arg_size());
			std::transform(fn->arg_begin(), fn->arg_end(), args.begin(),
			               [](llvm::Argument& arg) { return &arg; });

			if constexpr (std::convertible_to<std::invoke_result_t<Fn&&, std::span<llvm::Argument*>>,
			                                  llvm::Value*>) {
				const auto ret = std::invoke(std::forward<Fn>(body_builder), args);
				builder.CreateRet(ret);
			}
			else {
				std::invoke(std::forward<Fn>(body_builder), args);
				builder.CreateRetVoid();
			}

			if (dbg) _dib->finalizeSubprogram(dbg);
		}

		llvm::BasicBlock*
		entry() const {
			if (_entry) return _entry;
			_entry = define(fn->getContext());
			return _entry;
		}

		explicit(false) operator llvm::FunctionCallee() const noexcept {
			return {type, fn};
		}
	};
}

#endif
