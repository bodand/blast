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
 * src/c4rt2/src/c4rt2c/ast2_ir_emitter/emit_function_call --
 *   
 */

#include <numeric>

#include <c4/ast2/block.hxx>
#include <c4/ast2/expression.hxx>
#include <c4/ast2/float_literal.hxx>
#include <c4/ast2/fn_call.hxx>
#include <c4/ast2/integer_literal.hxx>
#include <c4/ast2/let_expression.hxx>
#include <c4/ast2/op_call.hxx>
#include <c4/ast2/string_literal.hxx>
#include <c4/ast2/symbol.hxx>

#include <c4rt2c/ast2_ir_emitter.hxx>
#include <c4rt2c/llvm_value_attribute.hxx>

#include <libassert/assert.hpp>

#include <lyra/main.hpp>

#include <llvm/Support/Casting.h>

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>

namespace {
	llvm::Value*
	try_get_value_attribute(const c4::ast2::tags::attributable* ref) {
		ASSERT(ref, "ref must not be null");

		const auto attr = ref->attribute_value<llvm::Value*>("value");
		if (!attr) return nullptr;

		return *attr;
	}
}

void
c4rt2c::ast2_ir_emitter::emit_function_call(
	const c4::ast2::symbol& symbol,
	std::span<const c4::ast2::expression*const> args
) {
	// llvm::Value* fn = resolve_referenced(symbol);
	// if (!fn) fn = try_get_value_attribute(symbol.references());
	//
	// if (_let_only) {
	// 	std::ranges::for_each(
	// 		args, [&](const auto& arg) {
	// 			arg->accept(*this);
	// 		}
	// 	);
	// 	return;
	// }
	//
	// ASSERT(fn, "function reference not resolved", symbol.name(), symbol.base_arity());
	// llvm::Value* thunk = nullptr;
	// bool need_argv = false;
	// if (const auto ref = symbol.references()) {
	// 	if (ref->attribute_value<bool>("thunk?")) thunk = fn;
	// 	if (!thunk) {
	// 		if (const auto attr = ref->attribute_value<llvm::Value*>("value")) {
	// 			thunk = *attr;
	// 		}
	// 	}
	// }
	//
	// if (!thunk) {
	// 	bool is_block = false;
	// 	if (const auto ref = symbol.references()) {
	// 		if (const auto attr = ref->attribute_value<bool>("is_block")) {
	// 			is_block = *attr;
	// 		}
	// 	}
	// 	need_argv = true;
	// 	if (is_block) {
	// 		thunk = _builder.CreateCall(_rt_make_datum_block, {fn});
	// 	}
	// 	else {
	// 		thunk = _builder.CreateCall(_rt_make_thunk, {fn});
	// 	}
	// }
	// set_last_callee(thunk);
	//
	// std::ranges::for_each(
	// 	args, [&](const auto& arg) {
	// 		arg->accept(*this);
	// 	}
	// );
	//
	// if (need_argv) {
	// 	llvm::Value* argv;
	// 	if (const auto callee = symbol.references()) {
	// 		// in-source defined functions
	// 		const auto csym = callee->attribute_value<std::vector<c4::ast2::tags::referable*>>("closure symbols");
	//
	// 		const auto context_count = csym.transform([](const auto& x) { return x.size(); })
	// 		                               .value_or(std::size_t{});
	// 		const auto args_count = callee->base_arity();
	//
	// 		argv = allocate_argv(context_count + args_count);
	// 		argv->setName({symbol.name(), "_argv"});
	//
	// 		std::size_t i = 0;
	// 		for (; i < context_count; ++i) {
	// 			const auto idx = _builder.CreateGEP(
	// 				llvm::PointerType::get(_context, 0), argv,
	// 				llvm::ConstantInt::get(_context, llvm::APInt(64, i)),
	// 				{symbol.name(), "_argv_args"}
	// 			);
	// 			const auto val = (*csym)[i]->attribute_value<llvm::Value*>("value");
	// 			ASSERT(val, "closure symbol value not found", symbol.name(), i);
	// 			_builder.CreateStore(*val, idx);
	// 		}
	// 		for (; i < context_count + args_count; ++i) {
	// 			const auto arg_i = i - context_count;
	// 			const auto arg = args[arg_i];
	// 			const auto val = arg->attribute_value<llvm::Value*>("value");
	// 			ASSERT(val, "value not found for expression", symbol.name(), symbol.base_arity(), arg_i);
	//
	// 			const auto idx = _builder.CreateGEP(
	// 				llvm::PointerType::get(_context, 0), argv,
	// 				llvm::ConstantInt::get(_context, llvm::APInt(64, i)),
	// 				{symbol.name(), "_argv_args"}
	// 			);
	// 			_builder.CreateStore(
	// 				*val,
	// 				idx);
	// 		}
	// 	}
	// 	else {
	// 		argv = allocate_argv(args.size());
	// 		argv->setName({symbol.name(), "_argv"});
	//
	// 		for (std::size_t i = 0; i < args.size(); ++i) {
	// 			const auto arg = args[i];
	// 			const auto val = arg->attribute_value<llvm::Value*>("value");
	// 			ASSERT(val, "value not found for expression", symbol.name(), symbol.base_arity(), i);
	// 			const auto idx = _builder.CreateGEP(
	// 				llvm::PointerType::get(_context, 0), argv,
	// 				llvm::ConstantInt::get(_context, llvm::APInt(64, i)),
	// 				{symbol.name(), "_argv_args"}
	// 			);
	// 			_builder.CreateStore(
	// 				*val,
	// 				idx);
	// 		}
	// 	}
	//
	// 	_builder.CreateCall(_rt_set_thunk_args, {thunk, argv});
	// }

	if (const auto expr = active_expression()) {
		// TODO: temporary nullptr to allow other code to rely on "value" being
		// 	defined
		const auto ptr_t = llvm::PointerType::get(_context, 0);
		expr->emplace_attribute<c4c::llvm_value_attribute>("value",
			llvm::ConstantPointerNull::get(ptr_t)
		);
	}
}
