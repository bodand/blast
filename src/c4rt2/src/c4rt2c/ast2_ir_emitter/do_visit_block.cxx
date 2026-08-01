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
 * src/c4rt2/src/c4rt2c/ast2_ir_emitter/do_visit_block --
 *   
 */

#include <iostream>
#include <numeric>
#include <utility>

#include <c4/ast2/block.hxx>
#include <c4/ast2/expression.hxx>
#include <c4/ast2/let_expression.hxx>

#include <c4/tags/attributable.hxx>

#include <c4rt2c/ast2_ir_emitter.hxx>
#include <c4rt2c/llvm_value_attribute.hxx>
#include <c4rt2c/scoped_scope.hxx>

#include <libassert/assert.hpp>

#include "already_thunk_attribute.hxx"

void
c4rt2c::ast2_ir_emitter::do_visit(const c4::ast2::block& obj) {
	if (obj.attribute_value<c4::ast2::let_expression*>("owner")) {
		return emit_named_function(obj);
	}
	const auto name = _name_manager.lambda_name();
	const auto decl = declare_function(name);
	decl->setLinkage(llvm::GlobalValue::PrivateLinkage);

	//
	{
		scoped_scope scope(_builder);
		_builder.SetInsertPoint(define(decl));

		obj.emplace_attribute<c4c::llvm_value_attribute>("argv", decl->getArg(0));
		obj.emplace_attribute<c4c::llvm_value_attribute>("K", decl->getArg(1));
		emit_named_function(obj);

		_builder.CreateRetVoid();
	}

	const auto expr = active_expression();
	ASSERT(expr);

	const auto datum_fn = _runtime.make_datum_block(decl);

	const auto closed_over = obj.effective_context_symbols();
	const auto argv_sz = closed_over.size() + *obj.invocable_with();

	const auto ptr_t = llvm::PointerType::get(_context, 0);
	const auto argv = _runtime.allocate_array(argv_sz, 8);
	std::ranges::for_each(closed_over, [&, this, i=0](const auto& sym) mutable {
		const auto ref = sym.references();
		ASSERT(ref);

		const auto val = ref->template attribute_value<llvm::Value*>("value");
		ASSERT(val, "symbol didn't get defined to value", sym.name(), sym);

		const auto addr = _builder.CreateGEP(ptr_t, argv, _builder.getInt64(i++));
		_builder.CreateAlignedStore(*val, addr, llvm::Align(8));
	});

	_runtime.set_thunk_args(datum_fn, argv, argv_sz);

	expr->emplace_attribute<c4c::llvm_value_attribute>(
		"value",
		datum_fn
	);
	expr->emplace_attribute<already_thunk_attribute>("thunk?");
}
