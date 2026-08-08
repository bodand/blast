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
 * Originally created: 2026-07-30.
 *
 * src/c4rt2/include/c4rt2c/seq_node --
 *   
 */
#ifndef BLAST_SEQ_NODE_HXX
#define BLAST_SEQ_NODE_HXX

#include <memory>

#include <llvm/IR/IRBuilder.h>

namespace c4::ast2 {
	struct expression;
}

namespace c4rt2c {
	struct ast2_ir_emitter;
	struct runtime_emitter;

	struct immediate_seq {
		llvm::Function* fn;
		llvm::Value* argv;
	};

	struct seq_node {
		virtual std::unique_ptr<seq_node>
		push(const c4::ast2::expression* val, ast2_ir_emitter& ir) = 0;

		virtual std::unique_ptr<seq_node>
		push(const c4::ast2::expression* val) = 0;

		virtual std::unique_ptr<seq_node>*
		last() { return nullptr; }

		virtual llvm::Value*
		build(llvm::IRBuilder<>& builder, runtime_emitter& rt) const = 0;

		virtual std::optional<immediate_seq>
		build_immediate(llvm::IRBuilder<>& builder,
		                runtime_emitter& ir) const = 0;

		virtual void
		build_call(llvm::IRBuilder<>& builder,
		           runtime_emitter& rt,
		           llvm::Value* K) const = 0;

		virtual ~seq_node() = default;
	};

	struct seq_pair final : seq_node {
		seq_pair(std::unique_ptr<seq_node>&& left,
		         std::unique_ptr<seq_node>&& right);

		std::unique_ptr<seq_node>
		push(const c4::ast2::expression* val, ast2_ir_emitter& ir) override;

		std::unique_ptr<seq_node>
		push(const c4::ast2::expression* val) override;

		[[nodiscard]] std::unique_ptr<seq_node>*
		last() override;

		llvm::Value*
		build(llvm::IRBuilder<>& builder, runtime_emitter& rt) const override;

		std::optional<immediate_seq>
		build_immediate(llvm::IRBuilder<>& builder,
		                runtime_emitter& rt) const override;

		void
		build_call(llvm::IRBuilder<>& builder,
		           runtime_emitter& rt,
		           llvm::Value* K) const override;

	private:
		std::unique_ptr<seq_node> _left;
		std::unique_ptr<seq_node> _right;
	};

	struct seq_leaf final : seq_node {
		explicit seq_leaf(llvm::Value* value,
		                  llvm::Value* argv = nullptr);

		std::unique_ptr<seq_node>
		push(const c4::ast2::expression* val, ast2_ir_emitter& ir) override;

		std::unique_ptr<seq_node>
		push(const c4::ast2::expression* val) override;

		llvm::Value*
		build(llvm::IRBuilder<>& builder, runtime_emitter& rt) const override;

		std::optional<immediate_seq>
		build_immediate(llvm::IRBuilder<>& builder,
		                runtime_emitter& rt) const override;

		void
		build_call(llvm::IRBuilder<>& builder,
		           runtime_emitter& rt,
		           llvm::Value* K) const override;

	private:
		llvm::Value* _value;
		llvm::Value* _argv;
	};

	struct seq_tail_leaf final : seq_node {
		explicit
		seq_tail_leaf(llvm::Value* value,
		              llvm::Value* argv = nullptr)
			: _value(value)
			, _argv(argv) { }

		std::unique_ptr<seq_node>
		push(const c4::ast2::expression* val, ast2_ir_emitter& ir) override;

		std::unique_ptr<seq_node>
		push(const c4::ast2::expression* val) override;

		llvm::Value*
		build(llvm::IRBuilder<>& builder, runtime_emitter& rt) const override;

		std::optional<immediate_seq>
		build_immediate(llvm::IRBuilder<>& builder,
		                runtime_emitter& rt) const override;

		void
		build_call(llvm::IRBuilder<>& builder,
		           runtime_emitter& rt,
		           llvm::Value* K) const override;

	private:
		// states:
		//		 literal: _argv is not set -> pass _value directly to K
		//		 tail calls: _argv is set -> do tail call to _value(_argv_, K)

		llvm::Value* _value;
		llvm::Value* _argv;
	};

	struct seq_nil final : seq_node {
		std::unique_ptr<seq_node>
		push(const c4::ast2::expression* val, ast2_ir_emitter& ir) override;

		std::unique_ptr<seq_node>
		push(const c4::ast2::expression* val) override;

		llvm::Value*
		build(llvm::IRBuilder<>& builder, runtime_emitter& rt) const override;

		std::optional<immediate_seq>
		build_immediate(llvm::IRBuilder<>& builder,
		                runtime_emitter& rt) const override { return {}; }

		void
		build_call(llvm::IRBuilder<>& builder,
		           runtime_emitter& rt,
		           llvm::Value* K) const override;
	};
}

#endif
