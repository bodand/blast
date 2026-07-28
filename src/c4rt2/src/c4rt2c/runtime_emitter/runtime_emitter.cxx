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
	, _rt_allocate_array{
		make_rt_function(
			&_module,
			"_c4_allocate_array",
			llvm::PointerType::get(_context, 0),
			llvm::IntegerType::get(_context, 64),
			llvm::IntegerType::get(_context, 64))
	}
	, _rt_complete_thunk{
		make_rt_function(
			&_module,
			"_c4_complete_thunk",
			llvm::Type::getVoidTy(_context),
			llvm::PointerType::get(_context, 0),
			llvm::PointerType::get(_context, 0))
	}
	, _rt_evaluate{
		make_rt_function(
			&_module,
			"_c4_evaluate",
			llvm::Type::getVoidTy(_context),
			llvm::PointerType::get(_context, 0),
			llvm::PointerType::get(_context, 0))
	}
	, _rt_make_datum_block{
		make_rt_function(
			&_module,
			"_c4_make_datum_block",
			llvm::PointerType::get(_context, 0),
			llvm::PointerType::get(_context, 0))
	}
	, _rt_make_datum_float64{
		make_rt_function(
			&_module,
			"_c4_make_datum_float64",
			llvm::PointerType::get(_context, 0),
			builder.getFloatTy())
	}
	, _rt_make_datum_int64{
		make_rt_function(
			&_module,
			"_c4_make_datum_int64",
			llvm::PointerType::get(_context, 0),
			llvm::IntegerType::get(_context, 64))
	}
	, _rt_make_datum_str{
		make_rt_function(
			&_module,
			"_c4_make_datum_str",
			llvm::PointerType::get(_context, 0),
			llvm::PointerType::get(_context, 0),
			llvm::IntegerType::get(_context, 64))
	}
	, _rt_make_thunk{
		make_rt_function(
			&_module,
			"_c4_make_thunk",
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
	, _rt_seq2{
		make_rt_function(
			&_module,
			"_c4_seq2",
			llvm::Type::getVoidTy(_context),
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
	, _gc_malloc{
		make_rt_function(
			&_module,
			"GC_malloc",
			llvm::PointerType::get(_context, 0),
			llvm::IntegerType::get(_context, 64))
	} {
	constexpr auto datum_type_int64 = 0;
	constexpr auto datum_type_thunk = 1;
	constexpr auto datum_type_immediate = 2;

	const auto int32_t = llvm::IntegerType::get(_context, 32);
	const auto int64_t = llvm::IntegerType::get(_context, 64);
	const auto ptr_t = llvm::PointerType::get(_context, 0);

	constexpr auto datum_field_type = 0;
	constexpr auto datum_field_value = 2;
	constexpr auto datum_field_argv = 3;
	const auto datum_t = llvm::StructType::create(_context, "c4_datum_t");
	datum_t->setBody({
		int32_t, // type
		int32_t, // ???
		ptr_t,   // value
		ptr_t    // argv ptr
	});

	const auto get_datum_type = [&](llvm::Value* d) {
		const auto addr = builder.CreateStructGEP(datum_t, d, datum_field_type, {d->getName(), ".type.addr"});
		return builder.CreateLoad(int32_t, addr, {d->getName(), ".type"});
	};

	const auto get_datum_value = [&](llvm::Value* d, llvm::Type* type) {
		const auto addr = builder.CreateStructGEP(datum_t, d, datum_field_value, {d->getName(), ".value.addr"});
		return builder.CreateLoad(type, addr, {d->getName(), ".value"});
	};

	const auto get_datum_argv = [&](llvm::Value* d) {
		const auto addr = builder.CreateStructGEP(datum_t, d, datum_field_argv, {d->getName(), ".argv.addr"});
		return builder.CreateLoad(ptr_t, addr, {d->getName(), ".argv"});
	};

	const auto set_datum_type = [&](llvm::Value* d, llvm::Value* type) {
		const auto addr = builder.CreateStructGEP(datum_t, d, datum_field_type, {d->getName(), ".type.addr"});
		builder.CreateAlignedStore(type, addr, llvm::Align(8));
	};

	const auto set_datum_value = [&](llvm::Value* d, llvm::Value* value) {
		const auto addr = builder.CreateStructGEP(datum_t, d, datum_field_value, {d->getName(), ".value.addr"});
		builder.CreateAlignedStore(value, addr, llvm::Align(8));
	};

	const auto set_datum_argv = [&](llvm::Value* d, llvm::Value* argv) {
		const auto addr = builder.CreateStructGEP(datum_t, d, datum_field_argv, {d->getName(), ".argv.addr"});
		builder.CreateAlignedStore(argv, addr, llvm::Align(8));
	};

	constexpr auto completion_field_self = 0;
	constexpr auto completion_field_thunk = 1;
	constexpr auto completion_field_K = 2;
	const auto completion_t = llvm::StructType::create(_context, "c4_completion_t");
	completion_t->setBody({
		ptr_t, // self
		ptr_t, // thunk
		ptr_t  // K
	});

	const auto get_completion_self = [&](llvm::Value* c) {
		const auto addr = builder.CreateStructGEP(completion_t, c, completion_field_self, {c->getName(), ".self.addr"});
		return builder.CreateLoad(ptr_t, addr, {c->getName(), ".self"});
	};
	std::ignore = get_completion_self;

	const auto get_completion_thunk = [&](llvm::Value* c) {
		const auto addr = builder.CreateStructGEP(completion_t, c, completion_field_thunk, {c->getName(), ".thunk.addr"});
		return builder.CreateLoad(ptr_t, addr, {c->getName(), ".thunk"});
	};

	const auto get_completion_K = [&](llvm::Value* c) {
		const auto addr = builder.CreateStructGEP(completion_t, c, completion_field_K, {c->getName(), ".K.addr"});
		return builder.CreateLoad(ptr_t, addr, {c->getName(), ".K"});
	};

	const auto set_completion_self = [&](llvm::Value* c, llvm::Value* self) {
		const auto addr = builder.CreateStructGEP(completion_t, c, completion_field_self, {c->getName(), ".self.addr"});
		builder.CreateAlignedStore(self, addr, llvm::Align(8));
	};

	const auto set_completion_thunk = [&](llvm::Value* c, llvm::Value* thunk) {
		const auto addr = builder.CreateStructGEP(completion_t, c, completion_field_thunk, {c->getName(), ".thunk.addr"});
		builder.CreateAlignedStore(thunk, addr, llvm::Align(8));
	};

	const auto set_completion_K = [&](llvm::Value* c, llvm::Value* K) {
		const auto addr = builder.CreateStructGEP(completion_t, c, completion_field_K, {c->getName(), ".K.addr"});
		builder.CreateAlignedStore(K, addr, llvm::Align(8));
	};

	const auto tail_call = [&](llvm::Value* callee, llvm::Value* args, llvm::Value* K) {
		const auto call = builder.CreateCall(_function_type, callee, {args, K});
		call->setCallingConv(llvm::CallingConv::Tail);
		call->setTailCallKind(llvm::CallInst::TCK_MustTail);
	};

	const auto tail_callt = [&](const llvm::FunctionCallee callee,
	                            llvm::Value* args,
	                            llvm::Value* K) {
		const auto call = builder.CreateCall(callee, {args, K});
		call->setCallingConv(llvm::CallingConv::Tail);
		call->setTailCallKind(llvm::CallInst::TCK_MustTail);
	};

	const auto trap = llvm::Intrinsic::getDeclaration(&module, llvm::Intrinsic::trap);

	_rt_allocate_array.define(_context, builder, [&](const std::span<llvm::Argument*> args) {
		args[0]->setName("count");
		args[1]->setName("size");
		const auto cmp = builder.CreateICmp(llvm::CmpInst::ICMP_EQ, args[0], builder.getInt64(0));

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
			const auto mul = builder.CreateNUWMul(args[0], args[1]);
			const auto memory = allocate(mul);
			return memory;
		}
	});

	_rt_complete_thunk.define(_context, builder, [&](const std::span<llvm::Argument*> args) {
		const auto& self = args[0];
		self->setName("self");
		const auto& res = args[1];
		res->setName("resume");

		const auto target_thunk = get_completion_thunk(self);
		const auto target_K = get_completion_K(self);

		set_datum_type(target_thunk, get_datum_type(res));
		set_datum_value(target_thunk, get_datum_value(res, int64_t));
		set_datum_argv(target_thunk, get_datum_argv(res));

		const auto K = builder.CreateAlignedLoad(ptr_t, target_K, llvm::Align(8), "K");

		tail_call(K, target_K, res);
	});

	_rt_evaluate.define(_context, builder, [&](const std::span<llvm::Argument*> args) {
		const auto& thunk = args[0];
		thunk->setName("thunk");
		const auto& K = args[1];
		K->setName("K");

		const auto eval_done = llvm::BasicBlock::Create(_context, "rt_eval_done", _rt_evaluate.fn);
		const auto eval_thunk = llvm::BasicBlock::Create(_context, "rt_eval_thunk", _rt_evaluate.fn);
		const auto error = llvm::BasicBlock::Create(_context, "rt_error", _rt_evaluate.fn);

		const auto thunk_type = get_datum_type(thunk);

		const auto switch_ = builder.CreateSwitch(thunk_type, error, 2); {
			const auto ip = builder.saveIP();
			builder.SetInsertPoint(error);

			builder.CreateCall(trap);
			builder.CreateUnreachable();

			builder.restoreIP(ip);
		}

		switch_->addCase(builder.getInt32(datum_type_int64), eval_done); {
			const auto ip = builder.saveIP();
			builder.SetInsertPoint(eval_done);

			const auto cont_fn = builder.CreateAlignedLoad(ptr_t, K, llvm::Align(8), "cont_fn");
			tail_call(cont_fn, K, thunk);
			builder.CreateRetVoid();

			builder.restoreIP(ip);
		}

		switch_->addCase(builder.getInt32(datum_type_thunk), eval_thunk); {
			// deliberately not saving ip; define(..., { }) adds ret void here
			builder.SetInsertPoint(eval_thunk);

			set_datum_type(thunk, builder.getInt32(datum_type_immediate));

			const auto completer = allocate(8 + 8 + 8);
			completer->setName("completer");

			set_completion_self(completer, _rt_complete_thunk.fn);
			set_completion_thunk(completer, thunk);
			set_completion_K(completer, K);

			const auto func = get_datum_value(thunk, ptr_t);
			const auto argv = get_datum_argv(thunk);

			tail_call(func, argv, completer);
		}
	});

	_rt_make_datum_int64.define(_context, builder, [&](const std::span<llvm::Argument*> args) {
		args[0]->setName("ival");
		const auto memory = allocate(4 + 4 + 8 + 8);
		memory->setName("datum");

		set_datum_type(memory, builder.getInt32(datum_type_int64));
		set_datum_value(memory, args[0]);

		return memory;
	});

	_rt_make_thunk.define(_context, builder, [&](const std::span<llvm::Argument*> args) {
		args[0]->setName("callee");
		const auto memory = allocate(4 + 4 + 8 + 8);
		memory->setName("thunk");

		set_datum_type(memory, builder.getInt32(datum_type_thunk));
		set_datum_value(memory, args[0]);
		set_datum_argv(memory, llvm::ConstantPointerNull::get(ptr_t));

		return memory;
	});

	_rt_seq.define(_context, builder, [&](const std::span<llvm::Argument*> args) {
		llvm::Argument* thunks,* K;
		(thunks = args[0])->setName("thunks");
		(K = args[1])->setName("K");

		const auto left_addr = builder.CreateGEP(ptr_t, thunks, builder.getInt64(0), "left.addr");
		const auto left = builder.CreateLoad(ptr_t, left_addr, "left");

		const auto right_addr = builder.CreateGEP(ptr_t, thunks, builder.getInt64(1), "right.addr");
		const auto right = builder.CreateLoad(ptr_t, right_addr, "right");

		const auto cont = allocate(8 + 8 + 8);
		cont->setName("cont");

		set_completion_self(cont, _rt_seq2.fn);
		set_completion_thunk(cont, right);
		set_completion_K(cont, K);

		tail_callt(_rt_evaluate, left, cont);
	});

	_rt_seq2.define(_context, builder, [&](const std::span<llvm::Argument*> args) {
		llvm::Argument* self,* evaled;
		(self = args[0])->setName("self");
		(evaled = args[1])->setName("_evaled");
		std::ignore = evaled;

		const auto next = get_completion_thunk(self);
		const auto K = get_completion_K(self);

		tail_callt(_rt_evaluate, K, next);
	});

	_rt_set_thunk_args.define(_context, builder, [&](const std::span<llvm::Argument*> args) {
		args[0]->setName("datum");
		args[1]->setName("argv");
		set_datum_argv(args[0], args[1]);
	});
}
