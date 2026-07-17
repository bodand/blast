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
 * src/c4rt2/src/c4rt2c/ast2_ir_emitter/ast2_ir_emitter --
 *   
 */

#include <c4rt2c/ast2_ir_emitter.hxx>

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

c4rt2c::ast2_ir_emitter::ast2_ir_emitter(llvm::LLVMContext& context,
                                         llvm::Module& module,
                                         llvm::IRBuilder<llvm::ConstantFolder, llvm::IRBuilderDefaultInserter>& builder,
                                         llvm::FunctionPassManager& pass_manager,
                                         llvm::FunctionAnalysisManager& fna_manager)
	: _pass_manager{pass_manager}
	, _fna_manager{fna_manager}
	, _context{context}
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
			_builder.getFloatTy())
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
	, _rt_complete_thunk{
		make_rt_function(
			&_module,
			"_c4_complete_thunk",
			llvm::Type::getVoidTy(_context),
			llvm::PointerType::get(_context, 0),
			llvm::PointerType::get(_context, 0))
	} {
	const auto gc_malloc_ty = llvm::FunctionType::get(
		llvm::PointerType::get(_context, 0),
		{
			llvm::IntegerType::get(_context, 64)
		}, false);
	const auto gc_malloc = llvm::Function::Create(
		gc_malloc_ty,
		llvm::GlobalValue::ExternalLinkage,
		"GC_malloc",
		_module
	);

	constexpr auto datum_type_int64 = 0;
	constexpr auto datum_type_thunk = 1;
	constexpr auto datum_type_immediate = 2;

	const auto void_t = llvm::Type::getVoidTy(_context);
	const auto int32_t = llvm::IntegerType::get(_context, 32);
	const auto int64_t = llvm::IntegerType::get(_context, 64);
	const auto ptr_t = llvm::PointerType::get(_context, 0);

	const auto datum_t = llvm::StructType::create(context, "c4_datum_t");
	datum_t->setBody({int32_t, int32_t, ptr_t, ptr_t});

	const auto completion_t = llvm::StructType::create(context, "c4_completion_t");
	completion_t->setBody({ptr_t, ptr_t, ptr_t});

	const auto trap = llvm::Intrinsic::getDeclaration(&module, llvm::Intrinsic::trap);

	_rt_allocate_array.define(context, builder, [&](const std::span<llvm::Argument*> args) {
		const auto mul = builder.CreateNUWMul(args[0], args[1]);
		const auto memory = builder.CreateCall(gc_malloc, {mul});
		return memory;
	});

	_rt_make_datum_int64.define(context, builder, [&](const std::span<llvm::Argument*> args) {
		const auto memory = builder.CreateCall(gc_malloc, {builder.getInt64(4 + 4 + 8 + 8)});

		const auto type_ptr = builder.CreateStructGEP(datum_t, memory, 0, "datum_type");
		builder.CreateStore(builder.getInt32(datum_type_int64),
		                    type_ptr)->setAlignment(llvm::Align(8));

		const auto value_ptr = builder.CreateStructGEP(datum_t, memory, 2, "datum_value");
		builder.CreateStore(args[0], value_ptr)->setAlignment(llvm::Align(8));

		return memory;
	});

	_rt_make_thunk.define(context, builder, [&](const std::span<llvm::Argument*> args) {
		const auto memory = builder.CreateCall(gc_malloc, {builder.getInt64(4 + 4 + 8 + 8)});

		const auto type_ptr = builder.CreateStructGEP(datum_t, memory, 0, "datum_type");
		builder.CreateStore(builder.getInt32(datum_type_thunk),
		                    type_ptr)->setAlignment(llvm::Align(8));

		const auto value_ptr = builder.CreateStructGEP(datum_t, memory, 2, "datum_value");
		builder.CreateStore(args[0], value_ptr)->setAlignment(llvm::Align(8));

		const auto argv_ptr = builder.CreateStructGEP(datum_t, memory, 3, "datum_argv");
		builder.CreateStore(llvm::ConstantPointerNull::get(ptr_t), argv_ptr)->setAlignment(llvm::Align(8));

		return memory;
	});

	_rt_set_thunk_args.define(context, builder, [&](const std::span<llvm::Argument*> args) {
		const auto argv_ptr = builder.CreateStructGEP(datum_t, args[0], 3, "datum_argv");
		builder.CreateStore(args[1], argv_ptr)->setAlignment(llvm::Align(8));
	});

	_rt_complete_thunk.define(context, builder, [&](const std::span<llvm::Argument*> args) {
		const auto& self = args[0];
		const auto& res = args[1];

		const auto target_thunk_addr = builder.CreateStructGEP(completion_t, self, 1, "completion_thunk.addr");
		const auto target_thunk = builder.CreateLoad(ptr_t, target_thunk_addr, "completion_thunk");

		const auto target_K_addr = builder.CreateStructGEP(completion_t, self, 2, "completion_K.addr");
		const auto target_K = builder.CreateLoad(ptr_t, target_K_addr, "completion_K");

		const auto res_type_addr = builder.CreateStructGEP(datum_t, res, 0, "res_type.addr");
		const auto res_type = builder.CreateLoad(int32_t, res_type_addr, "res_type");

		const auto target_thunk_type_addr = builder.CreateStructGEP(datum_t, target_thunk, 0, "thunk_type.addr");
		builder.CreateAlignedStore(res_type, target_thunk_type_addr, llvm::Align(8));

		const auto res_value_addr = builder.CreateStructGEP(datum_t, res, 2, "res_value.addr");
		const auto res_value = builder.CreateLoad(int64_t, res_value_addr, "res_value");

		const auto target_thunk_value_addr = builder.CreateStructGEP(datum_t, target_thunk, 2, "thunk_value.addr");
		builder.CreateAlignedStore(res_value, target_thunk_value_addr, llvm::Align(8));

		const auto res_argv_addr = builder.CreateStructGEP(datum_t, res, 3, "res_argv.addr");
		const auto res_argv = builder.CreateLoad(ptr_t, res_argv_addr, "res_argv");

		const auto target_thunk_argv_addr = builder.CreateStructGEP(datum_t, target_thunk, 3, "thunk_argv.addr");
		builder.CreateAlignedStore(res_argv, target_thunk_argv_addr, llvm::Align(8));

		const auto K = builder.CreateAlignedLoad(ptr_t, target_K, llvm::Align(8), "K");

		const auto call = builder.CreateCall(_function_type, K, {target_K, res});
		call->setTailCallKind(llvm::CallInst::TCK_MustTail);
	});

	_rt_evaluate.define(context, builder, [&](const std::span<llvm::Argument*> args) {
		const auto& thunk = args[0];
		const auto& K = args[1];

		const auto eval_done = llvm::BasicBlock::Create(context, "rt_eval_done", _rt_evaluate.fn);
		const auto eval_thunk = llvm::BasicBlock::Create(context, "rt_eval_thunk", _rt_evaluate.fn);
		const auto error = llvm::BasicBlock::Create(context, "rt_error", _rt_evaluate.fn);

		const auto thunk_type_addr = builder.CreateStructGEP(datum_t, thunk, 0, "thunk_type.addr");
		const auto thunk_type = builder.CreateLoad(int32_t, thunk_type_addr, "thunk_type");

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
			const auto call = builder.CreateCall(_function_type, cont_fn, {K, thunk});
			call->setTailCallKind(llvm::CallInst::TCK_MustTail);
			builder.CreateRetVoid();

			builder.restoreIP(ip);
		}

		switch_->addCase(builder.getInt32(datum_type_thunk), eval_thunk); {
			// deliberately not saving ip; define(..., { }) adds ret void here
			builder.SetInsertPoint(eval_thunk);

			builder.CreateAlignedStore(builder.getInt32(datum_type_immediate), thunk_type_addr, llvm::Align(8));

			const auto completer = builder.CreateCall(gc_malloc_ty, gc_malloc, {builder.getInt64(8 + 8 + 8)}, "completer");

			const auto completer_func_addr = builder.CreateStructGEP(completion_t, completer, 0, "completer_fn.addr");
			builder.CreateAlignedStore(_rt_complete_thunk.fn, completer_func_addr, llvm::Align(8));

			const auto completer_target_addr = builder.
					CreateStructGEP(completion_t, completer, 1, "completer_target.addr");
			builder.CreateAlignedStore(thunk, completer_target_addr, llvm::Align(8));

			const auto completer_K_addr = builder.CreateStructGEP(completion_t, completer, 2, "completer_K.addr");
			builder.CreateAlignedStore(K, completer_K_addr, llvm::Align(8));

			const auto func_addr = builder.CreateStructGEP(datum_t, thunk, 2, "func.addr");
			const auto func = builder.CreateLoad(ptr_t, func_addr, "func");
			const auto argv_addr = builder.CreateStructGEP(datum_t, thunk, 3, "argv.addr");
			const auto argv = builder.CreateLoad(ptr_t, argv_addr, "argv");

			const auto call = builder.CreateCall(_function_type, func, {argv, completer});
			call->setTailCallKind(llvm::CallInst::TCK_MustTail);
		}
	});

	const auto c4_main_ty = llvm::FunctionType::get(
		llvm::IntegerType::get(_context, 32),
		{
			llvm::PointerType::get(_context, 0),
			llvm::PointerType::get(_context, 0)
		}, false
	);
	const auto c4_main = llvm::Function::Create(
		c4_main_ty,
		llvm::GlobalValue::ExternalLinkage,
		"_c4_main",
		_module
	);
	c4_main->getArg(0)->setName("argv");
	c4_main->getArg(1)->setName("K");
	_scopes.push_back(_name_manager.root());
	last_scope().continue_at(c4_main->getArg(1));
	_builder.SetInsertPoint(llvm::BasicBlock::Create(_context, "entry", c4_main));
	_last_callee_stack.push_back(nullptr);
}
