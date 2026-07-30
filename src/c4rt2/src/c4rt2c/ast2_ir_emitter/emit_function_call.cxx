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

#include <iostream>
#include <numeric>

#include <c4/ast2/block.hxx>
#include <c4/ast2/expression.hxx>
#include <c4/ast2/symbol.hxx>

#include <c4rt2c/ast2_ir_emitter.hxx>
#include <c4rt2c/llvm_value_attribute.hxx>

#include <libassert/assert.hpp>

#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>

#include "already_thunk_attribute.hxx"

void
c4rt2c::ast2_ir_emitter::emit_function_call(
	const c4::ast2::symbol& symbol,
	std::span<const c4::ast2::expression*const> args
) {
	const auto ptr_t = llvm::PointerType::get(_context, 0);

	auto [fresh, thunk] = thunked_symbol(symbol);

	llvm::Value* argv = nullptr;
	size_t closure_size = 0u;
	if (fresh) {
		size_t callee_args_size = args.size();
		std::optional<llvm::SmallVector<llvm::Value*, 4>> closure_over{};

		if (const auto ref = symbol.references()) {
			const auto raw_closures = ref->attribute_value<
				llvm::SmallVector<llvm::Value*, 4>>("closure-over");

			if (raw_closures) {
				closure_over = *raw_closures;
				callee_args_size += closure_over->size();
				closure_size += closure_over->size();
			}
		}

		argv = _runtime.allocate_array(callee_args_size, sizeof(void*));
		argv->setName({symbol.name(), ".argv"});

		if (closure_over) {
			for (size_t i = 0;
			     const auto& val : *closure_over) {
				const auto addr = _builder.CreateGEP(
					ptr_t,
					argv,
					llvm::ConstantInt::get(_context, llvm::APInt(64, i++)));
				_builder.CreateStore(val, addr);
			}
		}

		ASSERT(callee_args_size < UINT32_MAX, "way too much arguments for thunk");
		_runtime.set_thunk_args(thunk, argv, callee_args_size);
	}
	std::ranges::for_each(args, [&, i=closure_size](const auto& arg) mutable {
		arg->accept(*this);
		if (argv) {
			const auto addr = _builder.CreateGEP(
				ptr_t,
				argv,
				llvm::ConstantInt::get(_context, llvm::APInt(64, i++)));

			auto value = arg->template attribute_value<llvm::Value*>("value");
			ASSERT(value, "argument didn't get assigned a value", symbol.name(), arg);
			_builder.CreateStore(*value, addr);
		}
	});

	const auto expr = active_expression();
	ASSERT(expr);

	if (const auto ref = symbol.references()) {
		if (const auto needs = ref->attribute_value<bool>("needs-loading?");
			needs && *needs) {
			thunk = _builder.CreateLoad(ptr_t, thunk, {symbol.name(), ".load"});
		}
	}

	expr->emplace_attribute<c4c::llvm_value_attribute>(
		"value",
		thunk
	);
	expr->emplace_attribute<already_thunk_attribute>("thunk?");
}
