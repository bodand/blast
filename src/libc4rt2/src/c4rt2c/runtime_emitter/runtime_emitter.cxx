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

#include <c4/ast2/ast_context.hxx>
#include <c4rt2c/runtime_emitter.hxx>
#include <c4rt2c/runtime_fn.hxx>

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>

#include "../../../../../vcpkg/buildtrees/llvm/src/org-18.1.6-e754cb1d0b.clean/llvm/include/llvm/ExecutionEngine/GenericValue.h"

namespace {
	struct void_tag { };

	struct ptr_tag { };

	struct double_tag { };

	template<size_t S>
	struct i_tag { };

	using i64_tag = i_tag<64>;
	using i32_tag = i_tag<32>;
	using i8_tag = i_tag<8>;

	template<class T>
	struct type;

	template<>
	struct type<void_tag> {
		static llvm::Type*
		get(llvm::LLVMContext& ctx) {
			return llvm::Type::getVoidTy(ctx);
		}
	};

	template<>
	struct type<ptr_tag> {
		static llvm::PointerType*
		get(llvm::LLVMContext& ctx) {
			return llvm::PointerType::get(ctx, 0);
		}
	};

	template<>
	struct type<double_tag> {
		static llvm::Type*
		get(llvm::LLVMContext& ctx) {
			return llvm::Type::getDoubleTy(ctx);
		}
	};

	template<size_t S>
	struct type<i_tag<S>> {
		static llvm::IntegerType*
		get(llvm::LLVMContext& ctx) {
			return llvm::IntegerType::get(ctx, S);
		}
	};

	constexpr auto void_ = type<void_tag>{};
	#define TYPE_void void_,
	constexpr auto ptr = type<ptr_tag>{};
	#define TYPE_ptr ptr,
	constexpr auto i64 = type<i64_tag>{};
	#define TYPE_i64 i64,
	constexpr auto i32 = type<i32_tag>{};
	#define TYPE_i32 i32,
	constexpr auto i8 = type<i8_tag>{};
	#define TYPE_i8 i8,
	constexpr auto double_ = type<double_tag>{};
	#define TYPE_double double_,

	template<class R, class... Args>
	c4rt2c::runtime_fn
	make_rt_function(llvm::Module* module,
	                 bool decl_only,
	                 type<R>,
	                 std::string_view name,
	                 type<Args>...) {
		const auto fn_type = llvm::FunctionType::get(
			type<R>::get(module->getContext()),
			{type<Args>::get(module->getContext())...},
			false
		);
		const auto fn = llvm::Function::Create(
			fn_type,
			llvm::Function::ExternalLinkage,
			name,
			module
		);

		return {fn_type, fn, decl_only};
	}
}

#define STRIPPER(...) __VA_OPT__(,) __VA_ARGS__ RPAREN }

#define PASS(x) x
#define LPAREN (
#define RPAREN )

#define str_I(x) #x
#define str(x) str_I(x)

#define cat_I(x, y) x ## y
#define cat(x, y) cat_I(x, y)

#define fn_II(_, fn) cat(_rt_, fn)
#define fn_I(t, fn) t, str(cat(_c4_, fn))
#define fn(tfn) PASS(fn_II LPAREN TYPE_##tfn RPAREN) { make_rt_function LPAREN &_module, decl_only_rt, PASS(fn_I LPAREN TYPE_##tfn RPAREN) STRIPPER

c4rt2c::runtime_emitter::runtime_emitter(llvm::LLVMContext& ctx,
                                         llvm::Module& module,
                                         llvm::IRBuilder<>& builder,
	                                      bool decl_only_rt)
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
	, fn(ptr allocate_array)(i64, i64)
	, fn(ptr allocate_array_debug)(i64, i64, ptr)
	, fn(void apply)(ptr, ptr)
	, fn(void apply2)(ptr, ptr)
	, fn(void complete_thunk)(ptr, ptr)
	, fn(void evaluate)(ptr, ptr)
	, fn(void force_args)(ptr, ptr)
	, fn(void force_args2)(ptr, ptr)
	, fn(ptr make_datum_block)(ptr)
	, fn(ptr make_datum_float64)(double_)
	, fn(ptr make_datum_int64)(i64)
	, fn(ptr make_datum_nil)()
	, fn(ptr make_datum_str)(ptr, i64)
	, fn(ptr make_thunk)(ptr)
	, fn(ptr make_one_shot)(ptr)
	, fn(void seq_ti)(ptr, ptr)
	, fn(void seq_ti2)(ptr, ptr)
	, fn(void seq_tt)(ptr, ptr)
	, fn(void seq_tt2)(ptr, ptr)
	, fn(void set_thunk_args)(ptr, ptr, i32)
	, fn(ptr merge_argv)(ptr, i32, ptr)
	, _gc_malloc{
		make_rt_function(&_module, true, ptr, "GC_malloc", i64)
	}
	, _gc_debug_malloc{
		make_rt_function(&_module, true, ptr, "GC_debug_malloc", i64, ptr, i32)
	} {
	int8_t = i8.get(_context);
	int32_t = i32.get(_context);
	int64_t = i64.get(_context);
	ptr_t = ptr.get(_context);

	datum_t = llvm::StructType::create(
		_context, {
			int32_t, // type
			int32_t, // argv_sz
			ptr_t,   // value
			ptr_t    // argv ptr
		}, "c4_datum_t");

	completion_t = llvm::StructType::create(
		_context, {
			ptr_t, // self
			ptr_t, // payload
			ptr_t  // K
		}, "c4_completion_t");

	args_force_t = llvm::StructType::create(
		_context, {
			int32_t, // to_force
			int32_t, // argv_sz
			ptr_t,   // native trampoline fn
			ptr_t    // argv ptr
		}, "c4_args_force_t");
}
