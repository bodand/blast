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
 * src/c4rt2/src/c4rt2c/runtime_emitter/runtime_emitter --
 *   
 */

#include <c4rt2c/runtime_emitter.hxx>
#include <c4rt2c/runtime_fn.hxx>

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

namespace {
	template<class... Args>
	c4rt2c::runtime_fn
	make_rt_function(llvm::Module* module,
	                 std::string_view name,
	                 llvm::Type* ret, Args&&... args) {
		const auto fn_type = llvm::FunctionType::get(
			ret,
			{std::forward<Args>(args)...},
			false
		);
		const auto fn = llvm::Function::Create(
			fn_type,
			llvm::Function::ExternalLinkage,
			name,
			module
		);

		return {fn_type, fn};
	}
}

c4rt2c::runtime_emitter::runtime_emitter(llvm::LLVMContext& ctx,
                                         llvm::Module& module,
                                         llvm::IRBuilder<>& builder)
	: _context{ctx}
	, _module{module}
	, _builder{builder}
	, _function_type{
		llvm::FunctionType::get(
			llvm::Type::getVoidTy(_context),
			std::array<llvm::Type*, 2>{
				llvm::PointerType::get(_context, 0),
				llvm::PointerType::get(_context, 0)
			},
			false)
	}
	, _rt_make_thunk{
		make_rt_function(
			&_module,
			"_c4_make_thunk",
			llvm::PointerType::get(_context, 0),
			llvm::PointerType::get(_context, 0))
	}
	, _rt_set_thunk_args{
		make_rt_function(
			&_module,
			"_c4_set_thunk_args",
			llvm::Type::getVoidTy(_context),
			llvm::PointerType::get(_context, 0),
			llvm::PointerType::get(_context, 0))
	}
	, _rt_make_datum_str{
		make_rt_function(
			&_module,
			"_c4_make_datum_str",
			llvm::PointerType::get(_context, 0),
			llvm::PointerType::get(_context, 0),
			llvm::IntegerType::get(_context, 64))
	}
	, _rt_make_datum_int64{
		make_rt_function(
			&_module,
			"_c4_make_datum_int64",
			llvm::PointerType::get(_context, 0),
			llvm::IntegerType::get(_context, 64))
	}
	, _rt_make_datum_float64{
		make_rt_function(
			&_module,
			"_c4_make_datum_float64",
			llvm::PointerType::get(_context, 0),
			builder.getFloatTy())
	}
	, _rt_make_datum_block{
		make_rt_function(
			&_module,
			"_c4_make_datum_block",
			llvm::PointerType::get(_context, 0),
			llvm::PointerType::get(_context, 0))
	}
	, _rt_allocate_array{
		make_rt_function(
			&_module,
			"_c4_allocate_array",
			llvm::PointerType::get(_context, 0),
			llvm::IntegerType::get(_context, 64),
			llvm::IntegerType::get(_context, 64))
	}
	, _rt_evaluate{
		make_rt_function(
			&_module,
			"_c4_evaluate",
			llvm::Type::getVoidTy(_context),
			llvm::PointerType::get(_context, 0),
			llvm::PointerType::get(_context, 0))
	}
	, _rt_seq{
		make_rt_function(
			&_module,
			"_c4_seq",
			llvm::Type::getVoidTy(_context),
			llvm::PointerType::get(_context, 0),
			llvm::PointerType::get(_context, 0))
	}
	, _rt_complete_thunk{
		make_rt_function(
			&_module,
			"_c4_complete_thunk",
			llvm::Type::getVoidTy(_context),
			llvm::PointerType::get(_context, 0),
			llvm::PointerType::get(_context, 0))
	} { }
