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

#include <string_view>

#include <c4/ast2/fwd.hxx>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/PassManager.h>

namespace llvm {
	class Module;
	class LLVMContext;
	class Function;
	class Type;
	class FunctionType;
	class Value;
	class BasicBlock;
}

namespace c4rt2c {
	struct runtime_fn {
		llvm::FunctionType* type;
		llvm::Function* fn;

		llvm::BasicBlock*
		define(llvm::LLVMContext& ctx) const;

		template<class Fn>
		void
		define(llvm::LLVMContext& ctx, llvm::IRBuilder<>& builder, Fn&& body_builder) const {
			const auto ip = builder.saveIP();
			builder.SetInsertPoint(define(ctx));

			std::vector<llvm::Argument*> args(fn->arg_size());
			std::transform(fn->arg_begin(), fn->arg_end(), args.begin(),
			               [](llvm::Argument& arg) { return &arg; });

			if constexpr (std::convertible_to<decltype(std::invoke(std::forward<Fn>(body_builder), args)),
			                                  llvm::Value*>) {
				const auto ret = std::invoke(std::forward<Fn>(body_builder), args);
				builder.CreateRet(ret);
			}
			else {
				std::invoke(std::forward<Fn>(body_builder), args);
				builder.CreateRetVoid();
			}

			builder.restoreIP(ip);
		}

		explicit(false) operator llvm::FunctionCallee() const noexcept {
			return llvm::FunctionCallee(type, fn);
		}
	};

	struct ast2_ir_emitter : c4::ast2::ast2_visitor {
		using builder_type = llvm::IRBuilder<>;

		ast2_ir_emitter(llvm::LLVMContext& context,
		                llvm::Module& module,
		                builder_type& builder,
		                llvm::FunctionPassManager& pass_manager,
		                llvm::FunctionAnalysisManager& fna_manager);

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

		void full() { _let_only = false; }
		bool _let_only{true};

		void finalize() const;

	private:
		llvm::FunctionPassManager& _pass_manager;
		llvm::FunctionAnalysisManager& _fna_manager;
		llvm::LLVMContext& _context;
		llvm::Module& _module;
		builder_type& _builder;

		std::vector<const c4::ast2::expression*> _expression_stack;
		std::vector<llvm::Value*> _last_callee_stack;

		const c4::ast2::expression*
		active_expression();

		void
		set_last_callee(llvm::Value* val);

		void
		emit_function_call(const c4::ast2::symbol& symbol,
		                   std::span<const c4::ast2::expression* const> args);


		struct name_manager;

		struct function_scope {
			function_scope(const function_scope&) = delete;

			function_scope&
			operator=(const function_scope&) = delete;

			void definition(
				llvm::BasicBlock* define,
				builder_type* ir_builder
			);

			void
			continue_at(llvm::Value* continuation) noexcept;

			[[nodiscard]] llvm::Value*
			continue_at() const noexcept { return _next_continuation; }

			function_scope(function_scope&& other) noexcept
				: _ip{other._ip}
				, _owning{std::exchange(other._owning, false)}
				, _qualified_name{std::exchange(other._qualified_name, "")}
				, _manager{std::exchange(other._manager, nullptr)}
				, _ir_builder{std::exchange(other._ir_builder, nullptr)}
				, _started_by{std::exchange(other._started_by, nullptr)}
				, _next_continuation{std::exchange(other._next_continuation, nullptr)} { }

			function_scope&
			operator=(function_scope&& other) noexcept {
				if (this == &other) return *this;
				_owning = std::exchange(other._owning, false);
				_manager = std::exchange(other._manager, nullptr);
				_qualified_name = std::exchange(other._qualified_name, _qualified_name);
				_ir_builder = std::exchange(other._ir_builder, nullptr);
				_started_by = std::exchange(other._started_by, nullptr);
				_ip = other._ip;
				_next_continuation = std::exchange(other._next_continuation, _next_continuation);
				return *this;
			}

			[[nodiscard]] std::string_view
			qualified_name() const noexcept { return _qualified_name; }

			[[nodiscard]] const c4::ast2::let_expression*
			started_by() const { return _started_by; }

			void
			start_function(llvm::Function* fn);

			~function_scope() {
				if (!_owning) return;

				_manager->pop();
				if (_ir_builder) _ir_builder->restoreIP(_ip);
			}

		private:
			friend struct name_manager;

			explicit function_scope(name_manager* manager,
			                        std::string qualified_name,
			                        const c4::ast2::let_expression* started_by);

			builder_type::InsertPoint _ip{};
			bool _owning{true};
			std::string _qualified_name;
			name_manager* _manager;
			builder_type* _ir_builder{};
			const c4::ast2::let_expression* _started_by;
			llvm::Value* _next_continuation{};
		};

		struct name_manager {
			[[nodiscard]] function_scope
			root();

			std::string
			qualify_name_globally();

			[[nodiscard("automatic scope keeper")]] function_scope
			push(const c4::ast2::let_expression& started_by,
			     const c4::ast2::symbol& name);

			[[nodiscard("automatic scope keeper")]] function_scope
			push_lambda(std::string name);

			void
			pop() noexcept;

			std::string
			string_name();

			std::string
			lambda_name();

			static std::string
			global_name(const c4::ast2::symbol& sym);

		private:
			std::size_t _string_counter{};
			std::size_t _lambda_counter{};
			std::vector<std::string> _names;
		} _name_manager{};

		llvm::Value*
		allocate_argv(std::size_t count) const;

		/// The universal function type to allow unrestricted
		/// tail-calls. It is void(ptr, ptr), where the first is an array
		/// to pointers as "argv" and the latter is the K continuation.
		llvm::FunctionType* _function_type;

		std::unordered_map<std::string, llvm::Function*> _extlib_functions{};
		std::unordered_map<std::string, llvm::Function*> _predeclared_functions{};

		runtime_fn _rt_make_thunk;
		runtime_fn _rt_set_thunk_args;
		runtime_fn _rt_make_datum_str;
		runtime_fn _rt_make_datum_int64;
		runtime_fn _rt_make_datum_float64;
		runtime_fn _rt_make_datum_block;
		runtime_fn _rt_allocate_array;
		runtime_fn _rt_evaluate;
		runtime_fn _rt_complete_thunk;

		std::vector<function_scope> _scopes;

		[[nodiscard]] function_scope&
		last_scope();

		[[nodiscard]] llvm::Function*
		declare_function(std::string_view name);

		[[nodiscard]] llvm::BasicBlock*
		define(llvm::Function* fn_decl);

		[[nodiscard]] llvm::Function*
		resolve_referenced(const c4::ast2::symbol& sym);
	};
}

#endif
