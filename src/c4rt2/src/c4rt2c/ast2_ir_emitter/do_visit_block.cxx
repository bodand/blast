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
#include <c4/tags/referable.hxx>

#include <c4rt2c/ast2_ir_emitter.hxx>
#include <c4rt2c/llvm_value_attribute.hxx>

#include <libassert/assert.hpp>

namespace {
	struct funk {
		llvm::Value* _val;
		const c4::ast2::tags::attributable* attr_holder;

		funk(const c4::ast2::tags::attributable* arg, llvm::Value* val)
			: _val(val)
			, attr_holder{arg} { }

		~funk() {
			attr_holder->emplace_attribute<c4c::llvm_value_attribute>("value", _val);
		}
	};

	struct seq_builder_node {
		virtual std::unique_ptr<seq_builder_node>
		push(llvm::Value* val) = 0;

		virtual std::unique_ptr<seq_builder_node>*
		last() { return nullptr; }

		virtual std::ostream&
		print(std::ostream& os) const = 0;

		virtual llvm::Value*
		build(llvm::IRBuilder<>& builder, c4rt2c::runtime_emitter& rt) const = 0;

		virtual ~seq_builder_node() = default;
	};

	struct seq_builder_seq final : seq_builder_node {
		seq_builder_seq(std::unique_ptr<seq_builder_node>&& left,
		                std::unique_ptr<seq_builder_node>&& right)
			: _left{std::move(left)}
			, _right{std::move(right)} { }

		std::unique_ptr<seq_builder_node>
		push(llvm::Value* val) override {
			return _right->push(val);
		}

		[[nodiscard]] std::unique_ptr<seq_builder_node>*
		last() override { return &_right; }

		std::ostream&
		print(std::ostream& os) const override {
			return _right->print(_left->print(os << "(") << ",") << ")";
		}

		llvm::Value*
		build(llvm::IRBuilder<>& builder, c4rt2c::runtime_emitter& rt) const override {
			const auto seq_left = _left->build(builder, rt);
			seq_left->setName("seql");
			const auto seq_right = _right->build(builder, rt);
			seq_right->setName("seqr");

			return rt.make_seq_thunk(seq_left, seq_right);
		}

		std::unique_ptr<seq_builder_node> _left;
		std::unique_ptr<seq_builder_node> _right;
	};

	struct seq_builder_leaf final : seq_builder_node {
		explicit seq_builder_leaf(llvm::Value* value)
			: _value{value} { }

		std::unique_ptr<seq_builder_node>
		push(llvm::Value* val) override {
			return std::make_unique<seq_builder_seq>(
				std::make_unique<seq_builder_leaf>(_value),
				std::make_unique<seq_builder_leaf>(val)
			);
		}

		llvm::Value*
		build(llvm::IRBuilder<>& builder, c4rt2c::runtime_emitter& rt) const override {
			return _value;
		}

		std::ostream&
		print(std::ostream& os) const override {
			return os << std::string_view(_value->getName());
		}

		llvm::Value* _value;
	};

	struct seq_builder_empty final : seq_builder_node {
		std::unique_ptr<seq_builder_node>
		push(llvm::Value* val) override {
			return std::make_unique<seq_builder_leaf>(val);
		}

		std::ostream&
		print(std::ostream& os) const override {
			return os << "()";
		}

		llvm::Value*
		build(llvm::IRBuilder<>& builder, c4rt2c::runtime_emitter& rt) const override {
			ASSERT(false, "WIP");
			return nullptr;
		}
	};

	struct seq_builder {
		seq_builder()
			: _root{std::make_unique<seq_builder_empty>()}
			, _last{&_root} { }

		void
		push(llvm::Value* val) {
			auto next = (*_last)->push(val);
			_last->swap(next);
			if (const auto push_to = (*_last)->last()) _last = push_to;
		}

		[[nodiscard]] llvm::Value*
		build(llvm::IRBuilder<>& builder, c4rt2c::runtime_emitter& rt) const {
			return _root->build(builder, rt);
		}

		std::unique_ptr<seq_builder_node> _root;
		std::unique_ptr<seq_builder_node>* _last;
	};
}

void
c4rt2c::ast2_ir_emitter::do_visit(const c4::ast2::block& obj) {
	const auto ptr_t = llvm::PointerType::get(_context, 0);
	size_t argv_i = 0;

	const auto argv = obj.attribute_value<llvm::Value*>("argv");
	const auto K = obj.attribute_value<llvm::Value*>("K");
	ASSERT(argv);
	ASSERT(K);

	std::vector<funk> restorer_holder;

	for (const auto& sym : obj.effective_context_symbols()) {
		const auto ref = sym.references();
		if (!ref) continue;

		const auto addr = _builder.CreateGEP(
			ptr_t,
			*argv,
			llvm::ConstantInt::get(_context, llvm::APInt(64, argv_i++)));
		const auto val = _builder.CreateLoad(ptr_t, addr, sym.name());

		if (const auto last = ref->attribute_value<llvm::Value*>("value"))
			restorer_holder.emplace_back(ref, *last);
		ref->emplace_attribute<c4c::llvm_value_attribute>("value", val);
	}

	if (const auto args = obj.args()) {
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
	const auto& expr = obj.expressions();
	std::ranges::for_each(expr, [&](const auto& expr) {
		expr->accept(*this);
		if (!expr->template attribute_value<bool>("thunk?")) return;

		const auto val = expr->template attribute_value<llvm::Value*>("value");
		ASSERT(val);
		builder.push(*val);
	});

	const auto seq = builder.build(_builder, _runtime);

	_runtime.evaluate(seq, *K);
}
