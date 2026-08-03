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
 * Originally created: 2026-08-03.
 *
 * src/c4rt2/include/c4rt2c/fn_call_gen --
 *   
 */
#ifndef BLAST_FN_CALL_GEN_HXX
#define BLAST_FN_CALL_GEN_HXX

#include <span>

#include <c4/ast2/symbol.hxx>

#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/IRBuilder.h>

namespace llvm {
	class Value;
}

namespace c4::ast2 {
	struct expression;

	namespace tags {
		struct referable;
	}}

namespace c4rt2c {
	struct runtime_emitter;
	struct ast2_ir_emitter;

	struct fn_call_gen {
		fn_call_gen(llvm::IRBuilder<>& builder,
		            runtime_emitter& rt,
		            const std::span<const c4::ast2::expression* const> args)
			: _builder{builder}
			, _rt{rt}
			, _args{args} { }

		virtual
		~fn_call_gen() noexcept = default;

		virtual llvm::Value*
		build(ast2_ir_emitter& ir, const c4::ast2::expression& expr) = 0;

	protected:
		[[nodiscard]] std::span<const c4::ast2::expression* const>
		args() const { return _args; }

		[[nodiscard]] std::size_t
		args_sz() const { return _args.size(); }

		llvm::IRBuilder<>& _builder;
		runtime_emitter& _rt;

	private:
		std::span<const c4::ast2::expression* const> _args;
	};

	struct direct_fn_call_gen : fn_call_gen {
		explicit
		direct_fn_call_gen(llvm::IRBuilder<>& builder,
		                   runtime_emitter& rt,
		                   const c4::ast2::symbol& symbol,
		                   std::span<const c4::ast2::expression* const> args)
			: fn_call_gen{builder, rt, args}
			, _symbol{symbol} { }

		llvm::Value*
		build(ast2_ir_emitter& ir, const c4::ast2::expression& expr) final;

	protected:
		[[nodiscard]] virtual llvm::Value*
		resolve_symbol_value(const c4::ast2::symbol& symbol) = 0;

		[[nodiscard]] c4::ast2::tags::referable*
		refer() const noexcept;

		virtual void
		finalize(llvm::Value* fn,
		         llvm::Value* argv,
		         std::size_t argv_sz,
		         const c4::ast2::expression& expr) = 0;

	private:
		template<std::output_iterator<llvm::Value*> It>
		void
		load_closures(It out) {
			const auto ref = _symbol.references();
			const auto closure_over = ref->attribute_value<
				llvm::SmallVector<llvm::Value*, 4>
			>("closure-over");

			if (!closure_over) return;

			_closure_args_sz = closure_over->size();
			std::ranges::copy(*closure_over, out);
		}

		c4::ast2::symbol _symbol;

		llvm::Value* _fn{};
		llvm::Value* _argv{};

		std::size_t _closure_args_sz{};
		std::size_t _argv_sz{};
	};

	struct immediate_fn_call_gen final : direct_fn_call_gen {
		explicit
		immediate_fn_call_gen(
			llvm::IRBuilder<>& builder,
			runtime_emitter& rt,
			const c4::ast2::symbol& symbol,
			const std::span<const c4::ast2::expression* const> args)
			: direct_fn_call_gen{builder, rt, symbol, args} { }

	protected:
		llvm::Value*
		resolve_symbol_value(const c4::ast2::symbol& symbol) override;

		void
		finalize(llvm::Value* fn,
		         llvm::Value* argv,
		         std::size_t argv_sz,
		         const c4::ast2::expression& expr) override;
	};

	struct tail_fn_call_gen : direct_fn_call_gen {
		explicit
		tail_fn_call_gen(
			llvm::IRBuilder<>& builder,
			runtime_emitter& rt,
			const c4::ast2::symbol& symbol,
			const std::span<const c4::ast2::expression* const> args)
			: direct_fn_call_gen{builder, rt, symbol, args} { }

	protected:
		[[nodiscard]] llvm::Value*
		resolve_symbol_value(const c4::ast2::symbol& symbol) override;

		void
		finalize(llvm::Value* fn,
		         llvm::Value* argv,
		         std::size_t argv_sz,
		         const c4::ast2::expression& expr) override;
	};
}


#endif
