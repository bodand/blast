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
 * src/c4rt2/include/c4rt2c/seq_builder --
 *   Helper utility to build and emit a block's seq chain.
 */
#ifndef BLAST_SEQ_BUILDER_HXX
#define BLAST_SEQ_BUILDER_HXX

#include <memory>

#include <llvm/IR/IRBuilder.h>

namespace c4::ast2 {
	struct expression;
}

namespace c4rt2c {
	struct ast2_ir_emitter;
	struct runtime_emitter;
	struct seq_node;
	struct seq_nil;

	struct seq_builder {
		seq_builder();

		void
		push(const c4::ast2::expression* expr, ast2_ir_emitter& ir);

		void
		push(const c4::ast2::expression* val);

		void
		build(llvm::IRBuilder<>& builder,
		      runtime_emitter& rt,
		      llvm::Value* K) const;

		~seq_builder();

	private:
		std::unique_ptr<seq_node> _root;
		std::unique_ptr<seq_node>* _last;
	};
}

#endif
