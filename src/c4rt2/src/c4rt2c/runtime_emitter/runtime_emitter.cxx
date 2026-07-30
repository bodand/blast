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
	constexpr auto ptr = type<ptr_tag>{};
	constexpr auto i64 = type<i64_tag>{};
	constexpr auto i32 = type<i32_tag>{};
	constexpr auto double_ = type<double_tag>{};

	template<class R, class... Args>
	c4rt2c::runtime_fn
	make_rt_function(llvm::Module* module,
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
	, _rt_allocate_array{
		make_rt_function(&_module, ptr, "_c4_allocate_array", i64, i64)
	}
	, _rt_apply{make_rt_function(&_module, void_, "_c4_apply", ptr, ptr)}
	, _rt_apply2{make_rt_function(&_module, void_, "_c4_apply2", ptr, ptr)}
	, _rt_complete_thunk{
		make_rt_function(&_module, void_, "_c4_complete_thunk", ptr, ptr)
	}
	, _rt_evaluate{make_rt_function(&_module, void_, "_c4_evaluate", ptr, ptr)}
	, _rt_make_datum_block{
		make_rt_function(&_module, ptr, "_c4_make_datum_block", ptr)
	}
	, _rt_make_datum_float64{
		make_rt_function(&_module, ptr, "_c4_make_datum_float64", double_)
	}
	, _rt_make_datum_int64{
		make_rt_function(&_module, ptr, "_c4_make_datum_int64", i64)
	}
	, _rt_make_datum_str{
		make_rt_function(&_module, ptr, "_c4_make_datum_str", ptr, i64)
	}
	, _rt_make_thunk{make_rt_function(&_module, ptr, "_c4_make_thunk", ptr)}
	, _rt_seq{make_rt_function(&_module, void_, "_c4_seq", ptr, ptr)}
	, _rt_seq2{make_rt_function(&_module, void_, "_c4_seq2", ptr, ptr)}
	, _rt_set_thunk_args{
		make_rt_function(&_module, void_, "_c4_set_thunk_args", ptr, ptr, i32)
	}
	, _rt_merge_argv{
		make_rt_function(&_module, ptr, "_c4_merge_argv", ptr, i32, ptr)
	}
	, _gc_malloc{
		make_rt_function(&_module, ptr, "GC_malloc", i64)
	} {
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
			ptr_t, // thunk
			ptr_t  // K
		}, "c4_completion_t");

	const auto trap = llvm::Intrinsic::getDeclaration(&module, llvm::Intrinsic::trap);

	_rt_allocate_array.define(_context, builder, [&](const std::span<llvm::Argument*> args) {
		const auto count = with_name(args[0], "count");
		const auto size = with_name(args[1], "size");

		const auto cmp = builder.CreateICmp(llvm::CmpInst::ICMP_EQ, count, builder.getInt64(0));

		const auto zero_bb = llvm::BasicBlock::Create(_context, "nonalloc", _rt_allocate_array.fn);
		const auto alloc_bb = llvm::BasicBlock::Create(_context, "alloc", _rt_allocate_array.fn);
		builder.CreateCondBr(cmp, zero_bb, alloc_bb);

		// do not allocate zero size
		{
			builder.SetInsertPoint(zero_bb);
			builder.CreateRet(llvm::ConstantPointerNull::get(ptr_t));
		}

		// allocate
		{
			builder.SetInsertPoint(alloc_bb);
			const auto mul = builder.CreateNUWMul(count, size);
			const auto memory = allocate(mul);
			return memory;
		}
	});

	_rt_apply.define(_context, builder, [&](const std::span<llvm::Argument*> args) {
		const auto argv = with_name(args[0], "argv");
		const auto K = with_name(args[1], "K");

		const auto thunk_addr = builder.CreateGEP(ptr_t, argv, builder.getInt64(0), "thunk.addr");
		const auto thunk = builder.CreateLoad(ptr_t, thunk_addr, "thunk");

		const auto argv_tail = builder.CreateGEP(ptr_t, argv, builder.getInt64(1), "argv.tail");

		const auto completer = with_name(allocate(8 + 8 + 8), "completer");
		set_completion_self(completer, _rt_apply2.fn);
		set_completion_K(completer, K);
		set_completion_thunk(completer, argv_tail);

		tail_call(_rt_evaluate, thunk, completer);
	});

	_rt_apply2.define(_context, _builder, [&](const std::span<llvm::Argument*> args) {
		const auto self = with_name(args[0], "self");
		const auto value = with_name(args[1], "value");

		const auto callee = get_datum_value(value, ptr_t);
		const auto dyn = with_name(make_thunk(callee), "dyn");

		const auto argv = get_datum_argv(value);
		const auto argv_sz = get_datum_argv_sz(value);

		const auto K = get_completion_K(self);
		const auto callee_args = get_completion_thunk(self); // hijacked ptr field

		const auto new_args = with_name(merge_argv(argv, argv_sz, callee_args), "new_args");

		set_thunk_args(dyn, new_args, argv_sz);

		tail_call(_rt_evaluate, dyn, K);
	});

	_rt_complete_thunk.define(_context, builder, [&](const std::span<llvm::Argument*> args) {
		const auto& self = with_name(args[0], "self");
		const auto& res = with_name(args[1], "resume");

		const auto target_thunk = get_completion_thunk(self);
		const auto target_K = get_completion_K(self);

		set_datum_type(target_thunk, get_datum_type(res));
		set_datum_value(target_thunk, get_datum_value(res, int64_t));
		set_datum_argv(target_thunk, get_datum_argv(res));

		const auto K = builder.CreateAlignedLoad(ptr_t, target_K, llvm::Align(8), "K");

		tail_call(K, target_K, res);
	});

	_rt_evaluate.define(_context, builder, [&](const std::span<llvm::Argument*> args) {
		const auto& thunk = with_name(args[0], "thunk");
		const auto& K = with_name(args[1], "K");

		const auto eval_done = llvm::BasicBlock::Create(_context, "rt_eval_done", _rt_evaluate.fn);
		const auto eval_thunk = llvm::BasicBlock::Create(_context, "rt_eval_thunk", _rt_evaluate.fn);
		const auto error = llvm::BasicBlock::Create(_context, "rt_error", _rt_evaluate.fn);

		const auto thunk_type = get_datum_type(thunk);

		const auto switch_ = builder.CreateSwitch(thunk_type, eval_done, 6); {
			const auto ip = builder.saveIP();
			builder.SetInsertPoint(eval_done);

			const auto cont_fn = builder.CreateAlignedLoad(ptr_t, K, llvm::Align(8), "cont_fn");
			tail_call(cont_fn, K, thunk);
			builder.CreateRetVoid();

			builder.restoreIP(ip);
		}

		switch_->addCase(builder.getInt32(datum_type_immediate), error); {
			const auto ip = builder.saveIP();
			builder.SetInsertPoint(error);

			builder.CreateCall(trap);
			builder.CreateUnreachable();

			builder.restoreIP(ip);
		}

		switch_->addCase(builder.getInt32(datum_type_thunk), eval_thunk); {
			// deliberately not saving ip; define(..., { }) adds ret void here
			builder.SetInsertPoint(eval_thunk);

			set_datum_type(thunk, builder.getInt32(datum_type_immediate));

			const auto completer = with_name(allocate(8 + 8 + 8), "completer");

			set_completion_self(completer, _rt_complete_thunk.fn);
			set_completion_thunk(completer, thunk);
			set_completion_K(completer, K);

			const auto func = get_datum_value(thunk, ptr_t);
			const auto argv = get_datum_argv(thunk);

			tail_call(func, argv, completer);
		}
	});

	_rt_make_datum_block.define(_context, builder, [&](const std::span<llvm::Argument*> args) {
		const auto val = with_name(args[0], "blk");

		const auto memory = with_name(allocate(4 + 4 + 8 + 8), "datum");

		set_datum_type(memory, builder.getInt32(datum_type_block));
		set_datum_value(memory, val);

		return memory;
	});

	_rt_make_datum_float64.define(_context, builder, [&](const std::span<llvm::Argument*> args) {
		const auto val = with_name(args[0], "fval");

		const auto memory = with_name(allocate(4 + 4 + 8 + 8), "datum");

		set_datum_type(memory, builder.getInt32(datum_type_float64));
		set_datum_value(memory, val);

		return memory;
	});

	_rt_make_datum_int64.define(_context, builder, [&](const std::span<llvm::Argument*> args) {
		const auto val = with_name(args[0], "ival");

		const auto memory = with_name(allocate(4 + 4 + 8 + 8), "datum");

		set_datum_type(memory, builder.getInt32(datum_type_int64));
		set_datum_value(memory, val);

		return memory;
	});

	_rt_make_datum_str.define(_context, builder, [&](const std::span<llvm::Argument*> args) {
		const auto val = with_name(args[0], "str");
		const auto val_sz = with_name(args[1], "str_sz");

		const auto memory = with_name(allocate(4 + 4 + 8 + 8), "datum");

		const auto zero_size = builder.CreateAdd(val_sz,
		                                         builder.getInt64(1),
		                                         "zero_size", true);
		const auto cpy = with_name(allocate(zero_size), "cpy");
		builder.CreateMemCpy(cpy, llvm::Align(1), val, llvm::Align(1), val_sz);

		const auto end = builder.CreateGEP(ptr_t, cpy, val_sz, "end");
		builder.CreateStore(builder.getInt8(0), end);

		set_datum_type(memory, builder.getInt32(datum_type_string));
		set_datum_value(memory, val);
		set_datum_argv(memory, val_sz);

		return memory;
	});

	_rt_make_thunk.define(_context, builder, [&](const std::span<llvm::Argument*> args) {
		const auto callee = with_name(args[0], "callee");
		const auto memory = with_name(allocate(4 + 4 + 8 + 8), "thunk");

		set_datum_type(memory, builder.getInt32(datum_type_thunk));
		set_datum_value(memory, callee);
		set_datum_argv(memory, llvm::ConstantPointerNull::get(ptr_t));

		return memory;
	});

	_rt_seq.define(_context, builder, [&](const std::span<llvm::Argument*> args) {
		const auto thunks = with_name(args[0], "thunks");
		const auto K = with_name(args[1], "K");

		const auto left_addr = builder.CreateGEP(ptr_t, thunks, builder.getInt64(0), "left.addr");
		const auto left = builder.CreateLoad(ptr_t, left_addr, "left");

		const auto right_addr = builder.CreateGEP(ptr_t, thunks, builder.getInt64(1), "right.addr");
		const auto right = builder.CreateLoad(ptr_t, right_addr, "right");

		const auto cont = with_name(allocate(8 + 8 + 8), "cont");

		set_completion_self(cont, _rt_seq2.fn);
		set_completion_thunk(cont, right);
		set_completion_K(cont, K);

		tail_call(_rt_evaluate, left, cont);
	});

	_rt_seq2.define(_context, builder, [&](const std::span<llvm::Argument*> args) {
		const auto self = with_name(args[0], "self");
		const auto evaled = with_name(args[1], "_evaled");
		std::ignore = evaled;

		const auto next = get_completion_thunk(self);
		const auto K = get_completion_K(self);

		tail_call(_rt_evaluate, K, next);
	});

	_rt_set_thunk_args.define(_context, builder, [&](const std::span<llvm::Argument*> args) {
		const auto datum = with_name(args[0], "datum");
		const auto argv = with_name(args[1], "argv");
		const auto argv_sz = with_name(args[2], "argv_sz");

		set_datum_argv(datum, argv);
		set_datum_argv_sz(datum, argv_sz);
	});

	_rt_merge_argv.define(_context, builder, [&](const std::span<llvm::Argument*> args_) {
		const auto argv = with_name(args_[0], "argv");
		const auto argv_sz = with_name(args_[1], "argv_sz");
		const auto args = with_name(args_[2], "args");

		const auto big_sz = _builder.CreateIntCast(argv_sz, int64_t, false, "big_sz");

		const auto out = with_name(allocate_array(
			                           big_sz,
			                           _builder.getInt64(8)
		                           ), "out");

		const auto init = llvm::BasicBlock::Create(_context, "init", _rt_merge_argv.fn);
		const auto loop1 = llvm::BasicBlock::Create(_context, "loop1", _rt_merge_argv.fn);
		const auto loop1c2 = llvm::BasicBlock::Create(_context, "loop1c2", _rt_merge_argv.fn);
		const auto loop1body = llvm::BasicBlock::Create(_context, "loop1body", _rt_merge_argv.fn);
		const auto loop2 = llvm::BasicBlock::Create(_context, "loop2", _rt_merge_argv.fn);
		const auto loop2body = llvm::BasicBlock::Create(_context, "loop2body", _rt_merge_argv.fn);
		const auto end = llvm::BasicBlock::Create(_context, "end", _rt_merge_argv.fn);

		_builder.CreateBr(init);
		_builder.SetInsertPoint(init);
		_builder.CreateBr(loop1);
		_builder.SetInsertPoint(loop1);

		const auto i = _builder.CreatePHI(int32_t, 2, "i");
		i->addIncoming(builder.getInt32(0), init);
		i->addIncoming(
			builder.CreateAdd(i, builder.getInt32(1), "i.loop", true),
			loop1body);

		const auto in_range = _builder.CreateICmp(llvm::CmpInst::ICMP_ULT, i, argv_sz);
		_builder.CreateCondBr(in_range, loop1c2, loop2);
		_builder.SetInsertPoint(loop1c2);

		const auto addr = _builder.CreateGEP(ptr_t, argv, i, "addr");
		const auto value = _builder.CreateAlignedLoad(ptr_t, addr, llvm::Align(8), "value");
		const auto not_null = _builder.CreateICmp(llvm::CmpInst::ICMP_NE, value, llvm::ConstantPointerNull::get(ptr_t));
		_builder.CreateCondBr(not_null, loop1body, loop2);
		_builder.SetInsertPoint(loop1body);

		const auto out_addr = _builder.CreateGEP(ptr_t, out, i, "out_addr");
		_builder.CreateAlignedStore(value, out_addr, llvm::Align(8));
		_builder.CreateBr(loop1);

		_builder.SetInsertPoint(loop2);
		const auto j = _builder.CreatePHI(int32_t, 3, "j");
		j->addIncoming(i, loop1);
		j->addIncoming(i, loop1c2);
		j->addIncoming(
			builder.CreateAdd(j, builder.getInt32(1), "j.loop", true),
			loop2body);

		const auto in_range2 = _builder.CreateICmp(llvm::CmpInst::ICMP_ULT, j, argv_sz);
		_builder.CreateCondBr(in_range2, loop2body, end);
		_builder.SetInsertPoint(loop2body);

		const auto argv_idx = _builder.CreateSub(j, i, "argv_idx", true);
		const auto argv_addr = _builder.CreateGEP(ptr_t, args, argv_idx, "argv_addr");
		const auto src = _builder.CreateAlignedLoad(ptr_t, argv_addr, llvm::Align(8), "src");
		const auto out2_addr = _builder.CreateGEP(ptr_t, out, j, "out2_addr");
		_builder.CreateAlignedStore(src, out2_addr, llvm::Align(8));
		_builder.CreateBr(loop2);

		_builder.SetInsertPoint(end);
		return out;
	});
}
