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
 * src/c4rt2/src/c4rt2c/ast2_ir_emitter/define_functoin --
 *   
 */

#include <c4/ast2/block.hxx>
#include <c4/ast2/expression.hxx>
#include <c4/ast2/let_expression.hxx>

#include <c4rt2c/ast2_ir_emitter.hxx>
#include <c4rt2c/llvm_value_attribute.hxx>
#include <c4rt2c/scoped_scope.hxx>

#include <libassert/assert.hpp>

namespace {
	struct closure_list_attribute : c4::ast2::tags::typed_attribute<llvm::SmallVector<llvm::Value*, 4>> {
		explicit
		closure_list_attribute(llvm::SmallVector<llvm::Value*, 4>&& val)
			: typed_attribute{val} { }
	};
}

void
c4rt2c::ast2_ir_emitter::define_function(const c4::ast2::let_expression& let) {
	DEBUG_ASSERT(let.introduces_function(), "let does not introduce fn", let);

	const auto fn = let.attribute_value<llvm::Function*>("function");
	ASSERT(fn, "function attribute must not be null", let.symbol());

	scoped_scope scope(_builder);
	_builder.SetInsertPoint(define(*fn));

	const auto body = let.function_body();
	body->emplace_attribute<c4c::llvm_value_attribute>("argv", (*fn)->getArg(0));
	body->emplace_attribute<c4c::llvm_value_attribute>("K", (*fn)->getArg(1));
	body->accept(*this);

	_builder.CreateRetVoid();

	if (const auto& stck = let.attribute_value<std::vector<c4::ast2::symbol>>(
		"symbol-stack")) {
		const auto src_name = _name_manager.format_symbols_stack(*stck);
		const auto meta_name = llvm::MDString::get(_context, src_name);
		const auto meta_node = llvm::MDNode::get(_context, meta_name);
		(*fn)->setMetadata("fn.source_name", meta_node);
	}

	if (const auto args = body->args()) {
		llvm::SmallVector<llvm::Metadata*, 4> arg_names;
		arg_names.reserve(args->block_arguments().size());

		for (const auto& arg : args->block_arguments()) {
			arg_names.push_back(llvm::MDString::get(_context, arg.name()));
		}

		const auto meta_node = llvm::MDNode::get(_context, arg_names);
		(*fn)->setMetadata("fn.args", meta_node);
	}

	if (const auto closures = let.value().closure_symbols();
		let.value().true_closure()) {
		llvm::SmallVector<llvm::Value*, 4> closure_args;
		llvm::SmallVector<llvm::Metadata*, 4> closure_names;
		closure_names.reserve(closures.size());

		for (const auto& closure : closures) {
			if (!closure.captured()) continue;

			const auto ref = closure.references();
			ASSERT(ref, "!captured closure symbol must have a reference",
			       closure.name(), closure.base_arity());

			const auto val = ref->attribute_value<llvm::Value*>("value");
			ASSERT(val, "closure symbol didn't get assigned a value",
			       closure.name(), closure.base_arity());
			closure_args.push_back(*val);

			closure_names.push_back(llvm::MDString::get(_context, closure.name()));
		}

		const auto meta_node = llvm::MDNode::get(_context, closure_names);
		(*fn)->setMetadata("fn.closure_args", meta_node);

		let.emplace_attribute<closure_list_attribute>("closure-over", std::move(closure_args));
	}
}
