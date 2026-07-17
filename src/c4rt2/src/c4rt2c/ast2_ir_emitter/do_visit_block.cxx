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

#include <numeric>
#include <utility>

#include <c4/ast2/block.hxx>
#include <c4/ast2/expression.hxx>
#include <c4/ast2/let_expression.hxx>

#include <c4/tags/attributable.hxx>
#include <c4/tags/referable.hxx>
#include <c4rt2c/ast2_ir_emitter.hxx>
#include <c4rt2c/llvm_value_attribute.hxx>

namespace {
	struct closure_symbols_attribute : c4::ast2::tags::typed_attribute<std::vector<c4::ast2::tags::referable*>> {
		explicit
		closure_symbols_attribute(const std::vector<c4::ast2::tags::referable*>& refs)
			: typed_attribute{refs} { }
	};

	struct already_thunk_attribute : c4::ast2::tags::typed_attribute<bool> {
		explicit
		already_thunk_attribute()
			: typed_attribute{true} { }
	};

	struct is_block_attribute : c4::ast2::tags::typed_attribute<bool> {
		explicit
		is_block_attribute()
			: typed_attribute{true} { }
	};

	struct lambda_name_attribute : c4::ast2::tags::typed_attribute<std::string> {
		explicit lambda_name_attribute(std::string name)
			: typed_attribute{std::move(name)} { }
	};
}

void
c4rt2c::ast2_ir_emitter::do_visit(const c4::ast2::block& obj) {
	// std::optional<function_scope> lambda_scope;
	// auto* scope_ptr = &last_scope();
	// if ((scope_ptr->started_by() && scope_ptr->started_by()->attribute_value<llvm::Function*>("function"))
	//     || (scope_ptr->started_by() && scope_ptr->started_by()->attribute_value<bool>("pre-declared"))) {
	// 	std::string name;
	// 	if (const auto attr = obj.attribute_value<std::string>("lambda name")) {
	// 		name = *attr;
	// 	}
	// 	else {
	// 		name = _name_manager.lambda_name();
	// 		obj.emplace_attribute<lambda_name_attribute>("lambda name", name);
	// 	}
	// 	lambda_scope.emplace(_name_manager.push_lambda(std::move(name)));
	// 	scope_ptr = &*lambda_scope;
	// }
	// auto& scope = *scope_ptr;
	//
	// auto expressions = obj.expressions();
	//
	// if (_let_only) {
	// 	declare_function(scope.qualified_name());
	// 	if (const auto start = scope.started_by()) {
	// 		start->emplace_attribute<already_thunk_attribute>("pre-declared");
	// 		start->emplace_attribute<is_block_attribute>("is_block");
	// 	}
	//
	// 	std::vector<c4::ast2::tags::referable*> closure_symbols;
	// 	for (const auto& sym : obj.effective_context_symbols()) {
	// 		const auto ref = sym.references();
	// 		if (!ref) continue;
	// 		if (ref == scope.started_by()) continue;
	//
	// 		closure_symbols.emplace_back(ref);
	// 	}
	// 	if (!closure_symbols.empty())
	// 		obj.emplace_attribute<closure_symbols_attribute>("closure symbols", closure_symbols);
	//
	// 	if (const auto args = obj.args()) {
	// 		for (auto& arg : args->block_arguments()) {
	// 			const_cast<c4::ast2::block_argument&>(arg).emplace_attribute<already_thunk_attribute>("thunk?");
	// 		}
	// 	}
	//
	// 	std::ranges::for_each(
	// 		expressions.begin(), expressions.end(), [&](const auto& expr) {
	// 			expr->accept(*this);
	// 		}
	// 	);
	// 	return;
	// }
	//
	// const auto fn = declare_function(scope.qualified_name());
	//
	// if (const auto expr = active_expression()) {
	// 	if (lambda_scope) {
	// 		const auto datum = _builder.CreateCall(_rt_make_datum_block, {fn}, "lambda.datum");
	// 		expr->emplace_attribute<c4c::llvm_value_attribute>("value", datum);
	// 		obj.emplace_attribute<is_block_attribute>("is_block");
	//
	// 		if (const auto attr = obj.attribute_value<std::vector<c4::ast2::tags::referable*>>("closure symbols")) {
	// 			const auto& csym = *attr;
	// 			if (!csym.empty()) {
	// 				const auto argv = allocate_argv(csym.size());
	// 				for (std::size_t i = 0; i < csym.size(); ++i) {
	// 					const auto idx = _builder.CreateGEP(
	// 						llvm::PointerType::get(_context, 0), argv,
	// 						llvm::ConstantInt::get(_context, llvm::APInt(64, i))
	// 					);
	// 					const auto val = csym[i]->attribute_value<llvm::Value*>("value");
	// 					if (val) {
	// 						_builder.CreateStore(*val, idx);
	// 					}
	// 				}
	// 				// _builder.CreateCall(_rt_set_thunk_args, {datum, argv});
	// 			}
	// 		}
	// 	}
	// 	else {
	// 		expr->emplace_attribute<c4c::llvm_value_attribute>("value", fn);
	// 	}
	// }
	//
	// std::vector<c4::ast2::tags::referable*> closure_symbols;
	// if (const auto attr = obj.attribute_value<std::vector<c4::ast2::tags::referable*>>("closure symbols")) {
	// 	closure_symbols = *attr;
	// }
	// else {
	// 	for (const auto& sym : obj.effective_context_symbols()) {
	// 		const auto ref = sym.references();
	// 		if (!ref) continue;
	// 		if (ref->attribute_value<llvm::Value*>("value")) continue;
	// 		if (ref == scope.started_by()) continue; // don't be a closure over oneself
	//
	// 		closure_symbols.emplace_back(ref);
	// 	}
	// 	if (!closure_symbols.empty())
	// 		obj.emplace_attribute<closure_symbols_attribute>("closure symbols", closure_symbols);
	// }
	//
	// if (const auto start = scope.started_by()) {
	// 	start->emplace_attribute<closure_symbols_attribute>("closure symbols", closure_symbols);
	// 	if (!fn->hasMetadata("fn_name")) {
	// 		std::string name(start->symbol().name());
	// 		name += "(";
	// 		name = std::accumulate(
	// 			closure_symbols.begin(), closure_symbols.end(),
	// 			name,
	// 			[](const auto& acc, const auto& sym) {
	// 				return acc + '^' + (sym ? std::string(sym->name()) : "??") + " ";
	// 			});
	// 		if (const auto args = obj.args()) {
	// 			name = std::accumulate(
	// 				args->block_arguments().begin(),
	// 				args->block_arguments().end(),
	// 				name,
	// 				[](const auto& acc, const auto& arg) {
	// 					return acc + std::string(arg.name()) + " ";
	// 				});
	// 		}
	// 		if (name.back() != ' ') {
	// 			name += ' ';
	// 		}
	// 		name.back() = ')';
	// 		auto meta_name = llvm::MDString::get(_context, name);
	// 		const auto md_tuple = llvm::MDTuple::get(_context, {meta_name});
	// 		fn->addMetadata("fn_name", *md_tuple);
	// 	}
	// }
	//
	// scope.start_function(fn);
	//
	// if (_let_only) {
	// 	return;
	// }
	//
	// scope.definition(define(fn), &_builder);
	//
	// const auto ptr = llvm::PointerType::get(_context, 0);
	// size_t i = 0;
	//
	// if (const auto csyms = obj.attribute_value<std::vector<c4::ast2::tags::referable*>>("closure symbols")) {
	// 	for (const auto& free_args : *csyms) {
	// 		const auto idx = _builder.CreateGEP(
	// 			ptr, fn->getArg(0),
	// 			llvm::ConstantInt::get(_context, llvm::APInt(64, i++)),
	// 			{free_args->name(), ".addr"}
	// 		);
	// 		const auto val = _builder.CreateLoad(ptr, idx, free_args->name());
	// 		free_args->emplace_attribute<c4c::llvm_value_attribute>("value", val);
	// 		free_args->emplace_attribute<already_thunk_attribute>("thunk?");
	// 	}
	// }
	//
	// if (obj.args()) {
	// 	for (auto& arg : obj.args()->block_arguments()) {
	// 		const_cast<c4::ast2::block_argument&>(arg).emplace_attribute<already_thunk_attribute>("thunk?");
	// 		const auto idx = _builder.CreateGEP(
	// 			ptr, fn->getArg(0),
	// 			llvm::ConstantInt::get(_context, llvm::APInt(64, i++)),
	// 			{arg.name(), ".addr"}
	// 		);
	// 		const auto val = _builder.CreateLoad(ptr, idx, arg.name());
	// 		arg.emplace_attribute<c4c::llvm_value_attribute>("value", val);
	// 		arg.emplace_attribute<already_thunk_attribute>("thunk?");
	// 	}
	// }
	//
	// _last_callee_stack.push_back(nullptr);
	// std::ranges::for_each(
	// 	expressions.rbegin(), expressions.rend(), [&](const auto& expr) {
	// 		expr->accept(*this);
	// 	}
	// );
	// const auto eval_start = _last_callee_stack.back();
	// _last_callee_stack.pop_back();
	//
	// if (!eval_start) {
	// 	const auto* expr = active_expression();
	// 	const auto res = expr ? expr->attribute_value<llvm::Value*>("value") : nullptr;
	// 	if (res) {
	// 		const auto eval = _builder.CreateCall(_rt_evaluate, {
	// 			                                      *res,
	// 			                                      scope.continue_at(),
	// 		                                      });
	// 		eval->setTailCallKind(llvm::CallInst::TCK_MustTail);
	// 		_builder.CreateRetVoid();
	// 		set_last_callee(*res);
	// 		return;
	// 	}
	// 	_builder.CreateRetVoid();
	// 	return;
	// }
	//
	// const auto eval = _builder.CreateCall(_rt_evaluate, {
	// 	                                      eval_start,
	// 	                                      scope.continue_at(),
	//                                       });
	// eval->setTailCallKind(llvm::CallInst::TCK_MustTail);
	// _builder.CreateRetVoid();
}
