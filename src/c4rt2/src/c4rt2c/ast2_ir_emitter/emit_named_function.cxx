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
 * Originally created: 2026-07-29.
 *
 * src/c4rt2/src/c4rt2c/ast2_ir_emitter/emit_named_function --
 *   
 */

#include <algorithm>
#include <iostream>

#include <c4/ast2/block.hxx>
#include <c4/ast2/expression.hxx>

#include <c4/tags/attributable.hxx>

#include <c4rt2c/ast2_ir_emitter.hxx>
#include <c4rt2c/llvm_value_attribute.hxx>
#include <c4rt2c/seq_builder.hxx>

#include <libassert/assert.hpp>

namespace {
	struct attr_restorer {
		llvm::Value* _val;
		const c4::ast2::tags::attributable* attr_holder;

		attr_restorer(const attr_restorer&) = delete;

		attr_restorer&
		operator=(const attr_restorer&) = delete;

		attr_restorer(attr_restorer&& mv) noexcept
			: _val{std::exchange(mv._val, nullptr)}
			, attr_holder{std::exchange(mv.attr_holder, nullptr)} { }

		attr_restorer&
		operator=(attr_restorer&& mv) noexcept {
			_val = std::exchange(mv._val, nullptr);
			attr_holder = std::exchange(mv.attr_holder, nullptr);
			return *this;
		}

		attr_restorer(const c4::ast2::tags::attributable* arg, llvm::Value* val)
			: _val(val)
			, attr_holder{arg} { }

		~attr_restorer() {
			if (!attr_holder) return;
			attr_holder->emplace_attribute<c4c::llvm_value_attribute>("value", _val);
		}
	};
}

void
c4rt2c::ast2_ir_emitter::emit_named_function(const c4::ast2::block& block) {
	const auto ptr_t = llvm::PointerType::get(_context, 0);
	size_t argv_i = 0;

	const auto argv = block.attribute_value<llvm::Value*>("argv");
	const auto K = block.attribute_value<llvm::Value*>("K");
	ASSERT(argv);
	ASSERT(K);

	std::vector<attr_restorer> restorer_holder;

	for (const auto& sym : block.effective_context_symbols()) {
		const auto ref = sym.references();
		ASSERT(ref);

		const auto addr = _builder.CreateGEP(
			ptr_t,
			*argv,
			llvm::ConstantInt::get(_context, llvm::APInt(64, argv_i++)));
		const auto val = _builder.CreateLoad(ptr_t, addr, sym.name());

		if (ref->attribute_value<llvm::Value*>("value"))
			restorer_holder.emplace_back(ref, *ref->attribute_value<llvm::Value*>("value"));
		ref->emplace_attribute<c4c::llvm_value_attribute>("value", val);
	}

	if (const auto args = block.args()) {
		for (const auto& arg : args->block_arguments()) {
			const auto addr = _builder.CreateGEP(
				ptr_t,
				*argv,
				llvm::ConstantInt::get(_context, llvm::APInt(64, argv_i++)));
			const auto val = _builder.CreateLoad(ptr_t, addr, arg.name());

			if (const auto last = arg.attribute_value<llvm::Value*>("value"))
				restorer_holder.emplace_back(&arg, *last);
			arg.emplace_attribute<c4c::llvm_value_attribute>("value", val);
		}
	}

	seq_builder builder;
	std::ranges::for_each(block.expressions(), [&](const auto& expr) {
		expr->accept(*this);
		const auto val = expr->template attribute_value<llvm::Value*>("value");
		if (!val) return;

		builder.push(*val);
	});

	const auto seq = builder.build(_builder, _runtime);

	_runtime.evaluate(seq, *K);
}
