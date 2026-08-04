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
 * Originally created: 2026-08-02.
 *
 * src/c4rt2/src/c4rt2c/runtime_emitter/init --
 *   
 */

#include <c4rt2c/runtime_emitter.hxx>

#include <llvm/ADT/APSInt.h>
#include <llvm/IR/DIBuilder.h>

void
c4rt2c::runtime_emitter::init(llvm::DIBuilder* dib,
                              llvm::DICompileUnit* cu) {
	const auto ptr_bits = _module.getDataLayout().getPointerSizeInBits(0);
	const auto ptr_align_bits = _module.getDataLayout().getPointerABIAlignment(0)
	                                   .value() * 8;

	const auto rt_file = dib->createFile("runtime", "narnia");

	auto* datum_fwd = dib->createReplaceableCompositeType(
		llvm::dwarf::DW_TAG_structure_type, "c4_datum_t", cu, rt_file, 0);
	auto* completion_fwd = dib->createReplaceableCompositeType(
		llvm::dwarf::DW_TAG_structure_type, "c4_completion_t", cu, rt_file, 0);
	auto* force_fwd = dib->createReplaceableCompositeType(
		llvm::dwarf::DW_TAG_structure_type, "c4_args_force_t", cu, rt_file, 0);

	const auto datum_ptr = dib->createPointerType(datum_fwd, ptr_bits, ptr_align_bits);
	const auto argv_ptr = dib->createPointerType(datum_ptr, ptr_bits, ptr_align_bits);
	const auto completion_ptr = dib->createPointerType(completion_fwd, ptr_bits, ptr_align_bits);
	const auto force_ptr = dib->createPointerType(force_fwd, ptr_bits, ptr_align_bits);

	// void(c4_completion_t *self, c4_datum_t *value)
	const auto cont_fn = dib->createPointerType(
		dib->createSubroutineType(
			dib->getOrCreateTypeArray({nullptr, completion_ptr, datum_ptr})), ptr_bits, ptr_align_bits);

	// void(c4_args_force_t *forces, c4_completion_t *K)
	const auto native_fn = dib->createPointerType(
		dib->createSubroutineType(
			dib->getOrCreateTypeArray({nullptr, force_ptr, completion_ptr})), ptr_bits, ptr_align_bits);

	const auto char_t = dib->createBasicType("char", CHAR_BIT, llvm::dwarf::DW_ATE_signed_char);

	_dbg_ptr_t = dib->createPointerType(nullptr, ptr_bits);
	_dbg_charptr_t = dib->createPointerType(char_t, ptr_bits);

	_dbg_int8_t = dib->createBasicType("int8_t", 8, llvm::dwarf::DW_ATE_signed);
	_dbg_uint32_t = dib->createBasicType("uint32_t", 32, llvm::dwarf::DW_ATE_unsigned);
	_dbg_int64_t = dib->createBasicType("int64_t", 64, llvm::dwarf::DW_ATE_signed);
	_dbg_uint64_t = dib->createBasicType("uint64_t", 64, llvm::dwarf::DW_ATE_unsigned);

	_dbg_float64_t = dib->createBasicType("float64_t", 64, llvm::dwarf::DW_ATE_float);

	_dbg_size_t = dib->createBasicType("size_t", 64, llvm::dwarf::DW_ATE_unsigned);
	_dbg_argv_size_t = dib->createBasicType("argv_size_t", 32, llvm::dwarf::DW_ATE_unsigned);

	auto enumerator = [&](const std::string_view name, const std::int64_t val) {
		return dib->createEnumerator(name, val, true);
	};

	auto* c4_type_enum_t = dib->createEnumerationType(
		cu, "c4_type_t", rt_file, 0,
		_dbg_uint32_t->getSizeInBits(), _dbg_uint32_t->getAlignInBits(),
		dib->getOrCreateArray({
			enumerator("C4_Integer", 0),
			enumerator("C4_Thunk", 1),
			enumerator("C4_Blackhole", 2),
			enumerator("C4_Float", 3),
			enumerator("C4_String", 4),
			enumerator("C4_Block", 5),
			enumerator("C4_Nil", 6),
		}),
		_dbg_uint32_t);


	const auto member = [&](const std::string_view name,
	                        llvm::DIType* t,
	                        const uint64_t off) {
		return dib->createMemberType(cu, name, rt_file, 0,
		                             t->getSizeInBits(),
		                             t->getAlignInBits(), off,
		                             llvm::DINode::FlagZero, t);
	};

	const auto cont_payload_t = dib->createUnionType(
		cu, "c4_cont_payload_t", rt_file, 0,
		_dbg_ptr_t->getSizeInBits(),
		_dbg_ptr_t->getAlignInBits(),
		llvm::DINode::FlagZero,
		dib->getOrCreateArray({
			member("thunk", datum_ptr, 0),
			member("args", argv_ptr, 0),
			member("forces", force_ptr, 0)
		})
	);

	const auto completion_layout = _module.getDataLayout().getStructLayout(completion_t);
	const auto dbg_completion_t = dib->createStructType(
		cu, "c4_completion_t", rt_file, 0,
		completion_layout->getSizeInBits(),
		completion_layout->getAlignment().value() * 8,
		llvm::DINode::FlagZero,
		nullptr,
		dib->getOrCreateArray({
			member("self", cont_fn, completion_layout->getElementOffsetInBits(0)),
			member("payload", cont_payload_t, completion_layout->getElementOffsetInBits(1)),
			member("K", completion_ptr, completion_layout->getElementOffsetInBits(2)),
		})
	);

	const auto args_force_layout = _module.getDataLayout().getStructLayout(args_force_t);
	const auto dbg_args_force_t = dib->createStructType(
		cu, "c4_args_force_t", rt_file, 0,
		args_force_layout->getSizeInBits(),
		completion_layout->getAlignment().value() * 8,
		llvm::DINode::FlagZero,
		nullptr, dib->getOrCreateArray({
			// countdown, mutated
			member("to_force", _dbg_argv_size_t, args_force_layout->getElementOffsetInBits(0)),
			// constant
			member("argv_sz", _dbg_argv_size_t, args_force_layout->getElementOffsetInBits(1)),
			member("native", native_fn, args_force_layout->getElementOffsetInBits(2)),
			member("argv", argv_ptr, args_force_layout->getElementOffsetInBits(3)),
		}));


	const auto datum_value_t = dib->createUnionType(
		cu, "", rt_file, 0,
		_dbg_ptr_t->getSizeInBits(),
		_dbg_ptr_t->getAlignInBits(),
		llvm::DINode::FlagZero,
		dib->getOrCreateArray({
			member("val_ptr", _dbg_ptr_t, 0),
			member("val_int64", _dbg_int64_t, 0),
			member("val_float64", _dbg_float64_t, 0),
		}));
	const auto datum_argv_t = dib->createUnionType(
		cu, "", rt_file, 0,
		_dbg_ptr_t->getSizeInBits(),
		_dbg_ptr_t->getAlignInBits(),
		llvm::DINode::FlagZero,
		dib->getOrCreateArray({
			member("argv", argv_ptr, 0),
			member("str_sz", _dbg_uint64_t, 0),
		})
	);

	const auto datum_layout = _module.getDataLayout().getStructLayout(datum_t);
	const auto c4_datum_t = dib->createStructType(
		cu, "c4_datum_t", rt_file, 0,
		datum_layout->getSizeInBits(),
		datum_layout->getAlignment().value() * 8,
		llvm::DINode::FlagZero,
		nullptr,
		dib->getOrCreateArray({
			member("type", c4_type_enum_t, datum_layout->getElementOffsetInBits(0)),
			member("argv_sz", _dbg_argv_size_t, datum_layout->getElementOffsetInBits(1)),
			member("", datum_value_t, datum_layout->getElementOffsetInBits(2)),
			member("", datum_argv_t, datum_layout->getElementOffsetInBits(3)),
		}));

	_gc_malloc.fn->addRetAttr(llvm::Attribute::NoAlias);
	_gc_malloc.fn->addFnAttr(llvm::Attribute::NoInline);
	_gc_malloc.fn->addFnAttr(llvm::Attribute::NoUnwind);
	_gc_malloc.fn->addFnAttr(llvm::Attribute::WillReturn);
	_gc_malloc.fn->addFnAttr(
		llvm::Attribute::getWithAllocSizeArgs(_context, 0, {})
	);

	constexpr auto allocation_flags = static_cast<std::int64_t>(
		llvm::AllocFnKind::Alloc | llvm::AllocFnKind::Zeroed
	);
	_gc_malloc.fn->addFnAttr(llvm::Attribute::get(
		_context, llvm::Attribute::AllocKind, allocation_flags
	));

	const auto trap = llvm::Intrinsic::getDeclaration(&_module, llvm::Intrinsic::trap);

	_rt_allocate_array.dbg(dib).file_scoped(rt_file)
	                  .name("_c4_allocate_array")
	                  .returns(_dbg_ptr_t)
	                  .arg("count", _dbg_size_t)
	                  .arg("size", _dbg_size_t);
	_rt_allocate_array.define(_builder, [&](const std::span<llvm::Argument*> args) {
		const auto count = args[0];
		const auto size = args[1];

		const auto cmp = _builder.CreateICmp(llvm::CmpInst::ICMP_EQ, count, _builder.getInt64(0));

		const auto zero_bb = llvm::BasicBlock::Create(_context, "nonalloc", _rt_allocate_array.fn);
		const auto alloc_bb = llvm::BasicBlock::Create(_context, "alloc", _rt_allocate_array.fn);
		_builder.CreateCondBr(cmp, zero_bb, alloc_bb);

		// do not allocate zero size
		{
			_builder.SetInsertPoint(zero_bb);
			_builder.CreateRet(llvm::ConstantPointerNull::get(ptr_t));
		}

		// allocate
		{
			_builder.SetInsertPoint(alloc_bb);
			const auto mul = _builder.CreateNUWMul(count, size);
			const auto memory = allocate(mul);
			return memory;
		}
	});
	_rt_allocate_array.fn->addRetAttr(llvm::Attribute::NoAlias);
	_rt_allocate_array.fn->addFnAttr(llvm::Attribute::WillReturn);
	_rt_allocate_array.fn->addFnAttr(
		llvm::Attribute::getWithAllocSizeArgs(_context, 1, 0)
	);
	_rt_allocate_array.fn->addFnAttr(llvm::Attribute::get(
		_context, llvm::Attribute::AllocKind, allocation_flags
	));

	_rt_apply.dbg(dib).file_scoped(rt_file)
	         .name("_c4_apply")
	         .arg("argv", argv_ptr)
	         .arg("K", completion_ptr);
	_rt_apply.define(_builder, [&](const std::span<llvm::Argument*> args) {
		const auto argv = args[0];
		const auto K = args[1];

		const auto thunk_addr = _builder.CreateGEP(ptr_t, argv, _builder.getInt64(0), "thunk.addr");
		const auto thunk = _builder.CreateLoad(ptr_t, thunk_addr, "thunk");

		const auto argv_tail = _builder.CreateGEP(ptr_t, argv, _builder.getInt64(1), "argv.tail");

		const auto completer = with_name(allocate(8 + 8 + 8), "completer");
		set_completion_self(completer, _rt_apply2.fn);
		set_completion_K(completer, K);
		set_completion_payload(completer, argv_tail);

		tail_call(_rt_evaluate, thunk, completer);
	});

	_rt_apply2.dbg(dib).file_scoped(rt_file)
	          .name("_c4_apply2")
	          .arg("self", completion_ptr)
	          .arg("value", datum_ptr);
	_rt_apply2.define(_builder, [&](const std::span<llvm::Argument*> args) {
		const auto self = args[0];
		const auto value = args[1];

		const auto K = get_completion_K(self);

		const auto apply = llvm::BasicBlock::Create(_context, "rt_apply", _rt_apply2.fn);
		const auto fallthrough = llvm::BasicBlock::Create(_context, "rt_fallthrough", _rt_apply2.fn);

		const auto type = get_datum_type(value);
		const auto is_block = _builder.CreateICmp(llvm::CmpInst::ICMP_EQ, type, _builder.getInt32(datum_type_block));
		_builder.CreateCondBr(is_block, apply, fallthrough);

		_builder.SetInsertPoint(fallthrough); {
			const auto cont_fn = _builder.CreateAlignedLoad(ptr_t, K, llvm::Align(8), "cont_fn");

			tail_call(cont_fn, K, value);
			_builder.CreateRetVoid();
		}

		_builder.SetInsertPoint(apply); { // apply actual block in datum
			const auto callee = get_datum_value(value, ptr_t);

			const auto argv = get_datum_argv(value);
			const auto argv_sz = get_datum_argv_sz(value);

			const auto callee_args = get_completion_payload(self); // hijacked ptr field

			const auto new_args = with_name(merge_argv(argv, argv_sz, callee_args), "new_args");

			tail_call(callee, new_args, K);
		}
	});

	_rt_complete_thunk.dbg(dib).file_scoped(rt_file)
	                  .name("_c4_complete_thunk")
	                  .arg("self", completion_ptr)
	                  .arg("value", datum_ptr);
	_rt_complete_thunk.define(_builder, [&](const std::span<llvm::Argument*> args) {
		const auto& self = args[0];
		const auto& res = args[1];

		const auto target_thunk = get_completion_payload(self);
		const auto target_K = get_completion_K(self);

		set_datum_type(target_thunk, get_datum_type(res));
		set_datum_value(target_thunk, get_datum_value(res, ptr_t));
		set_datum_argv(target_thunk, get_datum_argv(res));
		set_datum_argv_sz(target_thunk, get_datum_argv_sz(res));

		set_completion_payload(self, llvm::ConstantPointerNull::get(ptr_t));
		set_completion_K(self, llvm::ConstantPointerNull::get(ptr_t));

		const auto K = _builder.CreateAlignedLoad(ptr_t, target_K, llvm::Align(8), "K");

		tail_call(K, target_K, res);
	});

	_rt_evaluate.dbg(dib).file_scoped(rt_file)
	            .name("_c4_evaluate")
	            .arg("thunk", datum_ptr)
	            .arg("K", completion_ptr);
	_rt_evaluate.define(_builder, [&](const std::span<llvm::Argument*> args) {
		const auto& thunk = args[0];
		const auto& K = args[1];

		const auto eval_done = llvm::BasicBlock::Create(_context, "rt_eval_done", _rt_evaluate.fn);
		const auto eval_thunk = llvm::BasicBlock::Create(_context, "rt_eval_thunk", _rt_evaluate.fn);
		const auto error = llvm::BasicBlock::Create(_context, "rt_error", _rt_evaluate.fn);

		const auto thunk_type = get_datum_type(thunk);

		const auto switch_ = _builder.CreateSwitch(thunk_type, eval_done, 3); {
			const auto ip = _builder.saveIP();
			_builder.SetInsertPoint(eval_done);

			const auto cont_fn = _builder.CreateAlignedLoad(ptr_t, K, llvm::Align(8), "cont_fn");
			tail_call(cont_fn, K, thunk);
			_builder.CreateRetVoid();

			_builder.restoreIP(ip);
		}

		switch_->addCase(_builder.getInt32(datum_type_immediate), error); {
			const auto ip = _builder.saveIP();
			_builder.SetInsertPoint(error);

			_builder.CreateCall(trap);
			_builder.CreateUnreachable();

			_builder.restoreIP(ip);
		}

		switch_->addCase(_builder.getInt32(datum_type_thunk), eval_thunk); {
			_builder.SetInsertPoint(eval_thunk);

			set_datum_type(thunk, _builder.getInt32(datum_type_immediate));

			const auto completer = with_name(allocate(8 + 8 + 8), "completer");

			set_completion_self(completer, _rt_complete_thunk.fn);
			set_completion_payload(completer, thunk);
			set_completion_K(completer, K);

			const auto func = get_datum_value(thunk, ptr_t);
			const auto argv = get_datum_argv(thunk);

			tail_call(func, argv, completer);
		}
	});

	_rt_force_args.dbg(dib).file_scoped(rt_file)
	              .name("_c4_force_args")
	              .arg("forces", force_ptr)
	              .arg("K", completion_ptr);
	_rt_force_args.define(_builder, [&](const std::span<llvm::Argument*> args) {
		const auto forces = args[0];
		const auto K = args[1];

		const auto done = llvm::BasicBlock::Create(_context, "rt_force_done",
		                                           _rt_force_args.fn);
		const auto force_next = llvm::BasicBlock::Create(_context, "rt_force_next",
		                                                 _rt_force_args.fn);

		const auto rem_addr = _builder.CreateStructGEP(args_force_t,
		                                               forces,
		                                               args_force_field_to_force,
		                                               "forces.rem.addr");
		const auto rem = _builder.CreateLoad(int32_t, rem_addr, "rem");

		const auto zeroed = _builder.CreateICmp(llvm::CmpInst::ICMP_EQ, rem,
		                                        _builder.getInt32(0));
		_builder.CreateCondBr(zeroed, done, force_next);

		// remaining is zero -> done
		{
			_builder.SetInsertPoint(done);
			const auto native_addr = _builder.CreateStructGEP(args_force_t,
			                                                  forces,
			                                                  args_force_field_native,
			                                                  "forces.native.addr");
			const auto native_fn = _builder.CreateAlignedLoad(ptr_t, native_addr,
			                                                  llvm::Align(8),
			                                                  "forces.native");
			tail_call(native_fn, forces, K);
			_builder.CreateRetVoid();
		}

		// non-zero -> at least one thunk in argv should be evaled
		{
			_builder.SetInsertPoint(force_next);
			const auto cont = with_name(allocate(8 + 8 + 8), "cont");
			set_completion_self(cont, _rt_force_args2.fn);
			set_completion_K(cont, K);
			set_completion_payload(cont, forces);

			const auto argv_addr = _builder.CreateStructGEP(args_force_t, forces,
			                                                args_force_field_argv,
			                                                "forces.argv.addr");
			const auto argv = _builder.CreateAlignedLoad(ptr_t, argv_addr,
			                                             llvm::Align(8),
			                                             "forces.argv");

			const auto index = _builder.CreateSub(rem, _builder.getInt32(1), "argv.idx");
			const auto arg_addr = _builder.CreateGEP(ptr_t, argv, index, "arg.addr");

			const auto arg = _builder.CreateAlignedLoad(ptr_t, arg_addr,
			                                            llvm::Align(8), "arg");

			tail_call(_rt_evaluate, arg, cont);
			// implicit ret void
		}
	});

	_rt_force_args2.dbg(dib).file_scoped(rt_file)
	               .name("_c4_force_args2")
	               .arg("self", completion_ptr)
	               .arg("evaled", datum_ptr);
	_rt_force_args2.define(_builder, [&](const std::span<llvm::Argument*> args) {
		const auto self = args[0];
		const auto evaled = args[1];

		const auto K = get_completion_K(self);
		const auto forces = get_completion_payload(self);

		const auto rem_addr = _builder.CreateStructGEP(args_force_t,
		                                               forces,
		                                               args_force_field_to_force,
		                                               "forces.rem.addr");
		const auto rem = _builder.CreateAlignedLoad(int32_t, rem_addr,
		                                            llvm::Align(4), "rem");
		const auto index = _builder.CreateSub(rem, _builder.getInt32(1), "argv.idx");
		_builder.CreateAlignedStore(index, rem_addr, llvm::Align(4));

		const auto argv_addr = _builder.CreateStructGEP(args_force_t, forces,
		                                                args_force_field_argv,
		                                                "forces.argv.addr");
		const auto argv = _builder.CreateAlignedLoad(ptr_t, argv_addr,
		                                             llvm::Align(8),
		                                             "forces.argv");

		const auto arg_addr = _builder.CreateGEP(ptr_t, argv, index, "arg.addr");
		_builder.CreateAlignedStore(evaled, arg_addr, llvm::Align(8));

		tail_call(_rt_force_args, forces, K);
	});

	_rt_make_datum_block.dbg(dib).file_scoped(rt_file)
	                    .name("_c4_make_datum_block")
	                    .returns(datum_ptr)
	                    .arg("blk", _dbg_ptr_t);
	_rt_make_datum_block.define(_builder, [&](const std::span<llvm::Argument*> args) {
		const auto val = args[0];

		const auto memory = with_name(allocate(4 + 4 + 8 + 8), "datum");

		set_datum_type(memory, _builder.getInt32(datum_type_block));
		set_datum_value(memory, val);

		return memory;
	});

	_rt_make_datum_float64.dbg(dib).file_scoped(rt_file)
	                      .name("_c4_make_datum_float64")
	                      .returns(datum_ptr)
	                      .arg("fval", _dbg_float64_t);
	_rt_make_datum_float64.define(_builder, [&](const std::span<llvm::Argument*> args) {
		const auto val = args[0];

		const auto memory = with_name(allocate(4 + 4 + 8 + 8), "datum");

		set_datum_type(memory, _builder.getInt32(datum_type_float64));
		set_datum_value(memory, val);

		return memory;
	});

	_rt_make_datum_int64.dbg(dib).file_scoped(rt_file)
	                    .name("_c4_make_datum_int64")
	                    .returns(datum_ptr)
	                    .arg("ival", _dbg_int64_t);
	_rt_make_datum_int64.define(_builder, [&](const std::span<llvm::Argument*> args) {
		const auto val = args[0];

		const auto memory = with_name(allocate(4 + 4 + 8 + 8), "datum");

		set_datum_type(memory, _builder.getInt32(datum_type_int64));
		set_datum_value(memory, val);

		return memory;
	});

	_rt_make_datum_nil.dbg(dib).file_scoped(rt_file)
	                  .name("_c4_make_datum_nil")
	                  .returns(datum_ptr);
	_rt_make_datum_nil.define(_builder, [&](const std::span<llvm::Argument*> args) {
		const auto memory = with_name(allocate(4 + 4 + 8 + 8), "datum");

		set_datum_type(memory, _builder.getInt32(datum_type_nil));

		return memory;
	});

	_rt_make_datum_str.dbg(dib).file_scoped(rt_file)
	                  .name("_c4_make_datum_str")
	                  .returns(datum_ptr)
	                  .arg("str", _dbg_charptr_t)
	                  .arg("str_sz", _dbg_size_t);
	_rt_make_datum_str.define(_builder, [&](const std::span<llvm::Argument*> args) {
		const auto str = args[0];
		const auto str_sz = args[1];

		const auto memory = with_name(allocate(4 + 4 + 8 + 8), "datum");

		const auto zero_size = _builder.CreateAdd(str_sz,
		                                          _builder.getInt64(1),
		                                          "zero_size", true);
		const auto cpy = with_name(allocate(zero_size), "cpy");
		_builder.CreateMemCpy(cpy, llvm::Align(1), str, llvm::Align(1), str_sz);

		const auto end = _builder.CreateGEP(int8_t, cpy, str_sz, "end");
		_builder.CreateStore(_builder.getInt8(0), end);

		set_datum_type(memory, _builder.getInt32(datum_type_string));
		set_datum_value(memory, cpy);
		set_datum_argv(memory, str_sz);

		return memory;
	});

	_rt_make_thunk.dbg(dib).file_scoped(rt_file)
	              .name("_c4_make_thunk")
	              .returns(datum_ptr)
	              .arg("callee", _dbg_ptr_t);
	_rt_make_thunk.define(_builder, [&](const std::span<llvm::Argument*> args) {
		const auto callee = args[0];

		const auto memory = with_name(allocate(4 + 4 + 8 + 8), "thunk");

		set_datum_type(memory, _builder.getInt32(datum_type_thunk));
		set_datum_value(memory, callee);
		set_datum_argv(memory, llvm::ConstantPointerNull::get(ptr_t));

		return memory;
	});

	_rt_seq_tt.dbg(dib).file_scoped(rt_file)
	       .name("_c4_seq_tt")
	       .arg("thunks", argv_ptr)
	       .arg("K", completion_ptr);
	_rt_seq_tt.define(_builder, [&](const std::span<llvm::Argument*> args) {
		const auto thunks = args[0];
		const auto K = args[1];

		const auto left_addr = _builder.CreateGEP(ptr_t, thunks, _builder.getInt64(0), "left.addr");
		const auto left = _builder.CreateLoad(ptr_t, left_addr, "left");

		const auto right_addr = _builder.CreateGEP(ptr_t, thunks, _builder.getInt64(1), "right.addr");
		const auto right = _builder.CreateLoad(ptr_t, right_addr, "right");

		const auto cont = with_name(allocate(8 + 8 + 8), "cont");

		set_completion_self(cont, _rt_seq_tt2.fn);
		set_completion_payload(cont, right);
		set_completion_K(cont, K);

		tail_call(_rt_evaluate, left, cont);
	});

	_rt_seq_tt2.dbg(dib).file_scoped(rt_file)
	        .name("_c4_seq_tt2")
	        .arg("self", completion_ptr)
	        .arg("_evaled", datum_ptr);
	_rt_seq_tt2.define(_builder, [&](const std::span<llvm::Argument*> args) {
		const auto self = args[0];

		const auto next = get_completion_payload(self);
		const auto K = get_completion_K(self);

		tail_call(_rt_evaluate, next, K);
	});

	_rt_set_thunk_args.dbg(dib).file_scoped(rt_file)
	                  .name("_c4_set_thunk_args")
	                  .arg("datum", datum_ptr)
	                  .arg("argv", argv_ptr)
	                  .arg("argv_sz", _dbg_argv_size_t);
	_rt_set_thunk_args.define(_builder, [&](const std::span<llvm::Argument*> args) {
		const auto datum = args[0];
		const auto argv = args[1];
		const auto argv_sz = args[2];

		set_datum_argv(datum, argv);
		set_datum_argv_sz(datum, argv_sz);
	});

	_rt_merge_argv.dbg(dib).file_scoped(rt_file)
	              .name("_c4_merge_argv")
	              .returns(argv_ptr)
	              .arg("argv", argv_ptr)
	              .arg("argv_sz", _dbg_argv_size_t)
	              .arg("args", argv_ptr);
	_rt_merge_argv.define(_builder, [&](const std::span<llvm::Argument*> args_) {
		const auto argv = args_[0];
		const auto argv_sz = args_[1];
		const auto args = args_[2];

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
		i->addIncoming(_builder.getInt32(0), init);
		i->addIncoming(
			_builder.CreateAdd(i, _builder.getInt32(1), "i.loop", true),
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
			_builder.CreateAdd(j, _builder.getInt32(1), "j.loop", true),
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

	dib->replaceTemporary(llvm::TempDIType(completion_fwd), dbg_completion_t);
	dib->replaceTemporary(llvm::TempDIType(force_fwd), dbg_args_force_t);
	datum_fwd->replaceAllUsesWith(c4_datum_t);

	_di_builder = dib;
	_dbg_datum_ptr = datum_ptr;
}
