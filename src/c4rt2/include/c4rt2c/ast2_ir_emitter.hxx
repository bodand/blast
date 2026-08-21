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
 * Originally created: 2026-03-03.
 *
 * src/c4rt2/include/c4rt2c/ast3_ir_emitter --
 *   
 */
#ifndef BLAST_AST3_IR_EMITTER_HXX
#define BLAST_AST3_IR_EMITTER_HXX

#include <filesystem>
#include <optional>
#include <optional>
#include <string_view>

#include <c4/ast2/fwd.hxx>

#include <c4rt2c/runtime_emitter.hxx>
#include <c4rt2c/seq_builder.hxx>

#include <llvm/IR/DIBuilder.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/PassManager.h>

#include "fn_call_gen.hxx"

namespace c4::ast2::tags {
	struct tailable;
}

namespace llvm {
	class Module;
	class LLVMContext;
	class Function;
	class Type;
	class FunctionType;
	class Value;
	class BasicBlock;
	class DIBuilder;
}

namespace c4rt2c {
	struct ast2_ir_emitter : c4::ast2::ast2_visitor {
		using builder_type = llvm::IRBuilder<>;

		ast2_ir_emitter(llvm::Module& module,
		                builder_type& builder,
		                runtime_emitter&& rt,
		                const std::optional<std::string>& libinit_name);

		void do_visit(const c4::ast2::binary_op_call& obj) override;

		void do_visit(const c4::ast2::block& obj) override;

		void do_visit(const c4::ast2::dynamic_call& obj) override;

		void do_visit(const c4::ast2::expression& obj) override;

		void do_visit(const c4::ast2::float_literal& obj) override;

		void do_visit(const c4::ast2::fn_call& obj) override;

		void do_visit(const c4::ast2::integer_literal& obj) override;

		void do_visit(const c4::ast2::let_expression& obj) override;

		void do_visit(const c4::ast2::string_literal& obj) override;

		void do_visit(const c4::ast2::unary_op_call& obj) override;

		llvm::GlobalVariable*
		define_global_var(const c4::ast2::let_expression* gsym);

		void
		declare_symbols(const c4::ast2::ast_context& ctx);

		void
		init(const std::filesystem::path& fname);

		void
		finalize();

		llvm::Module&
		module() const noexcept { return _module; }

	private:
		llvm::LLVMContext& _context;
		llvm::Module& _module;
		builder_type& _builder;
		std::unique_ptr<llvm::DIBuilder> _di_builder;

		runtime_emitter _runtime;

		llvm::Function* _c4_main;
		llvm::Value* _mainK;
		seq_builder _seq_builder;

		std::vector<const c4::ast2::expression*> _expression_stack;

		llvm::DICompileUnit* _cu;
		llvm::DIFile* _file;

		const c4::ast2::expression*
		active_expression();

		void
		emit_function_call(fn_call_gen* builder);

		void
		emit_named_function(const c4::ast2::block& block);

		struct name_manager {
			std::string
			string_name();

			std::string
			lambda_name();

			static std::string
			global_name(const c4::ast2::symbol& sym);

			static std::string
			mangle_symbol(const c4::ast2::symbol& symbol);

			static std::string
			mangle_symbol_stack(std::span<const c4::ast2::symbol> symbols);

			static std::string
			format_symbols_stack(std::span<const c4::ast2::symbol> symbols);

		private:
			std::size_t _string_counter{};
			std::size_t _lambda_counter{};
		} _name_manager{};

		[[nodiscard]] llvm::Function*
		declare_function(std::string_view name);

		llvm::Function*
		declare_native_function(const c4::ast2::symbol& symbol);

		[[nodiscard]] std::pair<bool, llvm::Value*>
		thunked_symbol(const c4::ast2::symbol& sym) const;

		void
		define_bridge_function(const c4::ast2::let_expression& let);

		void
		define_function(const c4::ast2::let_expression& let);

		void
		define_variable(const c4::ast2::let_expression& let);

		[[nodiscard]] llvm::BasicBlock*
		define(llvm::Function* fn_decl);
	};
}

#endif
