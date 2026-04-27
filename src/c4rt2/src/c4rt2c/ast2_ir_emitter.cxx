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
 * Originally created: 2026-04-04.
 *
 * src/c4rt2/src/c4rt2c/ast2_ir_emitter --
 *   
 */

#include <numeric>

#include <c4/ast2/block.hxx>
#include <c4/ast2/expression.hxx>
#include <c4/ast2/float_literal.hxx>
#include <c4/ast2/fn_call.hxx>
#include <c4/ast2/integer_literal.hxx>
#include <c4/ast2/let_expression.hxx>
#include <c4/ast2/op_call.hxx>
#include <c4/ast2/string_literal.hxx>
#include <c4/ast2/symbol.hxx>

#include <c4rt2c/ast2_ir_emitter.hxx>
#include <c4rt2c/llvm_value_attribute.hxx>

#include <libassert/assert.hpp>

#include <lyra/main.hpp>

#include <llvm/Support/Casting.h>

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>

namespace {
	template<class... Args>
	llvm::FunctionCallee
	make_rt_function(llvm::Module* module,
	                 std::string_view name,
	                 llvm::Type* ret, Args&&... args) {
		const auto fn_type = llvm::FunctionType::get(
			ret,
			{std::forward<Args>(args)...},
			false
		);
		const auto fn = llvm::Function::Create(
			fn_type,
			llvm::Function::ExternalLinkage,
			name,
			module
		);

		return {fn_type, fn};
	}
}

c4rt2c::ast2_ir_emitter::ast2_ir_emitter(llvm::LLVMContext& context,
                                         llvm::Module& module,
                                         llvm::IRBuilder<llvm::ConstantFolder, llvm::IRBuilderDefaultInserter>& builder,
                                         llvm::FunctionPassManager& pass_manager,
                                         llvm::FunctionAnalysisManager& fna_manager)
	: _pass_manager{pass_manager}
	, _fna_manager{fna_manager}
	, _context{context}
	, _module{module}
	, _builder{builder}
	, _function_type{
		llvm::FunctionType::get(
			llvm::Type::getVoidTy(_context),
			std::array<llvm::Type*, 2>{
				llvm::PointerType::get(_context, 0),
				llvm::PointerType::get(_context, 0)
			},
			false)
	}
	, _rt_make_thunk{
		make_rt_function(
			&_module,
			"_c4_make_thunk",
			llvm::PointerType::get(_context, 0),
			llvm::PointerType::get(_context, 0),
			llvm::PointerType::get(_context, 0))
	}
	, _rt_set_thunk_args{
		make_rt_function(
			&_module,
			"_c4_set_thunk_args",
			llvm::Type::getVoidTy(_context),
			llvm::PointerType::get(_context, 0),
			llvm::PointerType::get(_context, 0))
	}
	, _rt_make_datum_str{
		make_rt_function(
			&_module,
			"_c4_make_datum_str",
			llvm::PointerType::get(_context, 0),
			llvm::PointerType::get(_context, 0),
			llvm::IntegerType::get(_context, 64))
	}
	, _rt_make_datum_int64{
		make_rt_function(
			&_module,
			"_c4_make_datum_int64",
			llvm::PointerType::get(_context, 0),
			llvm::IntegerType::get(_context, 64))
	}
	, _rt_make_datum_float64{
		make_rt_function(
			&_module,
			"_c4_make_datum_float64",
			llvm::PointerType::get(_context, 0),
			_builder.getFloatTy())
	}
	, _rt_evaluate{
		make_rt_function(
			&_module,
			"_c4_evaluate",
			llvm::Type::getVoidTy(_context),
			llvm::PointerType::get(_context, 0),
			llvm::PointerType::get(_context, 0))
	} {
	const auto c4_main_ty = llvm::FunctionType::get(
		llvm::IntegerType::get(_context, 32),
		{
			llvm::PointerType::get(_context, 0),
			llvm::PointerType::get(_context, 0)
		}, false
	);
	const auto c4_main = llvm::Function::Create(
		c4_main_ty,
		llvm::GlobalValue::ExternalLinkage,
		"_c4_main",
		_module
	);
	c4_main->getArg(0)->setName("argv");
	c4_main->getArg(1)->setName("K");
	_scopes.push_back(_name_manager.root());
	last_scope().continue_at(c4_main->getArg(1));
	_builder.SetInsertPoint(llvm::BasicBlock::Create(_context, "entry", c4_main));
	_last_callee_stack.push_back(nullptr);
}

void
c4rt2c::ast2_ir_emitter::do_visit(const c4::ast2::binary_op_call& obj) {
	emit_function_call(obj.op(), obj.args());
}

namespace {
	struct closure_symbols_attribute : c4::ast2::tags::typed_attribute<std::vector<c4::ast2::tags::referable*>> {
		explicit
		closure_symbols_attribute(const std::vector<c4::ast2::tags::referable*>& refs)
			: typed_attribute{refs} { }
	};
}

void
c4rt2c::ast2_ir_emitter::do_visit(const c4::ast2::block& obj) {
	auto& scope = last_scope();
	auto expressions = obj.expressions();
	const auto fn = declare_function(scope.qualified_name());

	if (const auto expr = active_expression()) {
		expr->emplace_attribute<c4c::llvm_value_attribute>("value", fn);
	}

	std::vector<c4::ast2::tags::referable*> closure_symbols;
	for (const auto& sym : obj.effective_context_symbols()) {
		const auto ref = sym.references();
		if (!ref) continue;
		if (ref->attribute_value<llvm::Value*>("value")) continue;
		if (ref == scope.started_by()) continue; // don't be a closure over oneself

		closure_symbols.emplace_back(ref);
	}
	if (!closure_symbols.empty())
		obj.emplace_attribute<closure_symbols_attribute>("closure symbols", closure_symbols);

	if (const auto start = scope.started_by()) {
		start->emplace_attribute<closure_symbols_attribute>("closure symbols", closure_symbols);
		if (!fn->hasMetadata("fn_name")) {
			std::string name(start->symbol().name());
			name += "(";
			name = std::accumulate(
				closure_symbols.begin(), closure_symbols.end(),
				name,
				[](const auto& acc, const auto& sym) {
					return acc + '^' + std::string(sym->name()) + " ";
				});
			if (const auto args = obj.args()) {
				name = std::accumulate(
					args->block_arguments().begin(),
					args->block_arguments().end(),
					name,
					[](const auto& acc, const auto& arg) {
						return acc + std::string(arg.name()) + " ";
					});
			}
			if (name.back() != ' ') {
				name += ' ';
			}
			name.back() = ')';
			auto meta_name = llvm::MDString::get(_context, name);
			const auto md_tuple = llvm::MDTuple::get(_context, {meta_name});
			fn->addMetadata("fn_name", *md_tuple);
		}
	}

	scope.start_function(fn);

	if (_let_only) {
		std::ranges::for_each(
			expressions.begin(), expressions.end(), [&](const auto& expr) {
				expr->accept(*this);
			}
		);
		return;
	}

	scope.definition(define(fn), &_builder);

	const auto ptr = llvm::PointerType::get(_context, 0);
	size_t i = 0;

	if (const auto csyms = obj.attribute_value<std::vector<c4::ast2::tags::referable*>>("closure symbols")) {
		for (const auto& free_args : *csyms) {
			const auto idx = _builder.CreateGEP(
				ptr, fn->getArg(0),
				llvm::ConstantInt::get(_context, llvm::APInt(64, i++)),
				{free_args->name(), ".addr"}
			);
			const auto val = _builder.CreateLoad(ptr, idx, free_args->name());
			free_args->emplace_attribute<c4c::llvm_value_attribute>("value", val);
		}
	}

	if (obj.args()) {
		for (auto& arg : obj.args()->block_arguments()) {
			const auto idx = _builder.CreateGEP(
				ptr, fn->getArg(0),
				llvm::ConstantInt::get(_context, llvm::APInt(64, i++)),
				{arg.name(), ".addr"}
			);
			const auto val = _builder.CreateLoad(ptr, idx, arg.name());
			arg.emplace_attribute<c4c::llvm_value_attribute>("value", val);
		}
	}

	_last_callee_stack.push_back(nullptr);
	std::ranges::for_each(
		expressions.rbegin(), expressions.rend(), [&](const auto& expr) {
			expr->accept(*this);
		}
	);
	const auto eval_start = _last_callee_stack.back();
	_last_callee_stack.pop_back();

	const auto eval = _builder.CreateCall(_rt_evaluate, {
		eval_start,
		llvm::ConstantPointerNull::get(llvm::PointerType::get(_context, 0))
	});
	eval->setTailCallKind(llvm::CallInst::TCK_MustTail);
	_builder.CreateRetVoid();
}

void
c4rt2c::ast2_ir_emitter::do_visit(const c4::ast2::dynamic_call& obj) {
	ASSERT(false, "sorry, not implemented");
}

void
c4rt2c::ast2_ir_emitter::do_visit(const c4::ast2::expression& obj) {
	_expression_stack.push_back(&obj);
	obj.accept_skip_self(*this);
	_expression_stack.pop_back();
}

void
c4rt2c::ast2_ir_emitter::do_visit(const c4::ast2::float_literal& obj) {
	if (_let_only) return;
	const auto val = _builder.CreateCall(
		_rt_make_datum_float64, {
			llvm::ConstantFP::get(_context, llvm::APFloat(obj.value()))
		},
		"lit.float"
	);
	if (const auto expr = active_expression()) {
		expr->emplace_attribute<c4c::llvm_value_attribute>("value", val);
	}
}

void
c4rt2c::ast2_ir_emitter::do_visit(const c4::ast2::let_expression& obj) {
	_scopes.emplace_back(_name_manager.push(obj, obj.symbol()));

	obj.value().accept(*this);

	_scopes.pop_back();
}

void
c4rt2c::ast2_ir_emitter::do_visit(const c4::ast2::string_literal& obj) {
	if (_let_only) return;
	const auto name = _name_manager.string_name();
	const auto bytes = _builder.CreateGlobalStringPtr(obj.value(), name);
	const auto bytes_sz = obj.value().size();

	const auto val = _builder.CreateCall(_rt_make_datum_str,
	                                     {bytes, _builder.getInt64(bytes_sz)},
	                                     "lit.str");
	if (const auto expr = active_expression()) {
		expr->emplace_attribute<c4c::llvm_value_attribute>("value", val);
	}
}

void
c4rt2c::ast2_ir_emitter::do_visit(const c4::ast2::unary_op_call& obj) {
	emit_function_call(obj.op(), obj.args());
}

void
c4rt2c::ast2_ir_emitter::finalize() const {
	const auto zero = llvm::ConstantInt::get(_context, llvm::APInt(32, 0));
	_builder.CreateRet(zero);
}

namespace {
	llvm::Function*
	try_get_function_attribute(const c4::ast2::tags::attributable* ref) {
		ASSERT(ref, "ref must not be null");

		const auto attr = ref->attribute_value<llvm::Function*>("function");
		if (!attr) return nullptr;

		return *attr;
	}

	llvm::Value*
	try_get_value_attribute(const c4::ast2::tags::attributable* ref) {
		ASSERT(ref, "ref must not be null");

		const auto attr = ref->attribute_value<llvm::Value*>("value");
		if (!attr) return nullptr;

		return *attr;
	}
}

void
c4rt2c::ast2_ir_emitter::do_visit(const c4::ast2::fn_call& obj) {
	emit_function_call(obj.sym(), obj.args());
}

void
c4rt2c::ast2_ir_emitter::do_visit(const c4::ast2::integer_literal& obj) {
	if (_let_only) return;
	const auto val = _builder.CreateCall(
		_rt_make_datum_int64, {
			llvm::ConstantInt::get(_context, llvm::APInt(64, obj.value()))
		},
		"lit.int"
	);
	if (const auto expr = active_expression()) {
		expr->emplace_attribute<c4c::llvm_value_attribute>("value", val);
	}
}

const c4::ast2::expression*
c4rt2c::ast2_ir_emitter::active_expression() {
	ASSERT(!_expression_stack.empty(),
	       "empty expression stack has no active expression");
	return _expression_stack.back();
}

void
c4rt2c::ast2_ir_emitter::set_last_callee(llvm::Value* val) {
	ASSERT(!_last_callee_stack.empty(), "callee stack must not be empty");
	_last_callee_stack.back() = val;
}

void
c4rt2c::ast2_ir_emitter::emit_function_call(
	const c4::ast2::symbol& symbol,
	std::span<const c4::ast2::expression*const> args
) {
	llvm::Value* fn = resolve_referenced(symbol);
	if (!fn) {
		fn = try_get_value_attribute(symbol.references());
	}

	if (_let_only) {
		std::ranges::for_each(
			args, [&](const auto& arg) {
				arg->accept(*this);
			}
		);
		return;
	}

	ASSERT(fn, "function reference not resolved", symbol.name(), symbol.base_arity());

	auto& scope = last_scope();
	auto* continue_at = scope.continue_at();
	const auto thunk = _builder.CreateCall(_rt_make_thunk, {fn, continue_at});
	scope.continue_at(thunk);
	set_last_callee(thunk);

	std::ranges::for_each(
		args, [&](const auto& arg) {
			arg->accept(*this);
		}
	);

	llvm::Value* argv;
	if (const auto callee = symbol.references()) {
		// in-source defined functions
		const auto csym = callee->attribute_value<std::vector<c4::ast2::tags::referable*>>("closure symbols");

		const auto context_count = csym.transform([](const auto& x) { return x.size(); })
		                               .value_or(std::size_t{});
		const auto args_count = callee->base_arity();

		const auto count_val = llvm::ConstantInt::get(_context,
		                                              llvm::APInt(64, context_count + args_count));
		argv = _builder.CreateAlloca(llvm::PointerType::get(_context, 0),
		                             count_val,
		                             {symbol.name(), "_argv"});

		std::size_t i = 0;
		for (; i < context_count; ++i) {
			const auto idx = _builder.CreateGEP(
				llvm::PointerType::get(_context, 0), argv,
				llvm::ConstantInt::get(_context, llvm::APInt(64, i)),
				{symbol.name(), "_argv_args"}
			);
			const auto val = (*csym)[i]->attribute_value<llvm::Value*>("value");
			ASSERT(val, "closure symbol value not found", symbol.name(), i);
			_builder.CreateStore(*val, idx);
		}
		for (; i < context_count + args_count; ++i) {
			const auto arg_i = i - context_count;
			const auto arg = args[arg_i];
			const auto val = arg->attribute_value<llvm::Value*>("value");
			ASSERT(val, "value not found for expression", symbol.name(), symbol.base_arity(), arg_i);

			const auto idx = _builder.CreateGEP(
				llvm::PointerType::get(_context, 0), argv,
				llvm::ConstantInt::get(_context, llvm::APInt(64, i)),
				{symbol.name(), "_argv_args"}
			);
			_builder.CreateStore(
				*val,
				idx);
		}
	}
	else {
		const auto count_val = llvm::ConstantInt::get(_context,
		                                              llvm::APInt(64, args.size()));
		argv = _builder.CreateAlloca(llvm::PointerType::get(_context, 0),
		                             count_val,
		                             {symbol.name(), "_argv"});
		for (std::size_t i = 0; i < args.size(); ++i) {
			const auto arg = args[i];
			const auto val = arg->attribute_value<llvm::Value*>("value");
			ASSERT(val, "value not found for expression", symbol.name(), symbol.base_arity(), i);
			const auto idx = _builder.CreateGEP(
				llvm::PointerType::get(_context, 0), argv,
				llvm::ConstantInt::get(_context, llvm::APInt(64, i)),
				{symbol.name(), "_argv_args"}
			);
			_builder.CreateStore(
				*val,
				idx);
		}
	}

	_builder.CreateCall(_rt_set_thunk_args, {fn, argv});
	if (const auto expr = active_expression()) {
		expr->emplace_attribute<c4c::llvm_value_attribute>("value", thunk);
	}
}

void
c4rt2c::ast2_ir_emitter::function_scope::definition(llvm::BasicBlock* define, builder_type* ir_builder) {
	_ir_builder = ir_builder;
	_ip = _ir_builder->saveIP();
	_ir_builder->SetInsertPoint(define);
}

void
c4rt2c::ast2_ir_emitter::function_scope::continue_at(llvm::Value* continuation) noexcept {
	ASSERT(continuation, "continuation must not be null");
	ASSERT(continuation->getType()->isPointerTy(), "continuation must be a pointer");

	_next_continuation = continuation;
}

void
c4rt2c::ast2_ir_emitter::function_scope::start_function(llvm::Function* fn) {
	if (!_started_by) return;

	continue_at(fn->getArg(1));
	_started_by->emplace_attribute<c4c::llvm_function_attribute>(
		"function",
		fn
	);
}

c4rt2c::ast2_ir_emitter::function_scope::function_scope(
	name_manager* manager,
	std::string qualified_name,
	const c4::ast2::let_expression* started_by
)
	: _qualified_name{std::move(qualified_name)}
	, _manager{manager}
	, _started_by{started_by} {
	ASSERT(!_qualified_name.empty(), "name must not be empty");
	ASSERT(_manager, "manager must not be null");
	if (_qualified_name == "_c4_main") {
		_owning = false;
	}
	else {
		ASSERT(_started_by, "started_by must not be null");
	}
}

c4rt2c::ast2_ir_emitter::function_scope
c4rt2c::ast2_ir_emitter::name_manager::root() {
	return function_scope{this, "_c4_main", nullptr};
}

std::string
c4rt2c::ast2_ir_emitter::name_manager::qualify_name_globally() {
	auto qualified_name = fmt::format("q{}S", _names.size());

	const auto full_size = qualified_name.size()
	                       + std::accumulate(
		                       _names.begin(), _names.end(), std::size_t{},
		                       [](auto acc, const auto& current_name) {
			                       return acc + current_name.size();
		                       }
	                       )
	                       + 1;
	qualified_name.reserve(full_size);

	qualified_name = std::accumulate(_names.begin(), _names.end(), qualified_name);
	qualified_name += 'E';
	return qualified_name;
}

c4rt2c::ast2_ir_emitter::function_scope
c4rt2c::ast2_ir_emitter::name_manager::push(const c4::ast2::let_expression& started_by,
                                            const c4::ast2::symbol& name) {
	_names.emplace_back(name.mangle());
	auto qualified_name = qualify_name_globally();

	return function_scope{this, std::move(qualified_name), &started_by};
}

void
c4rt2c::ast2_ir_emitter::name_manager::pop() noexcept {
	ASSERT(_names.size() >= 1, "global scope cannot pop name qualifier");

	_names.pop_back();
}

std::string
c4rt2c::ast2_ir_emitter::name_manager::string_name() {
	_names.emplace_back(fmt::format("str{}", _string_counter++));
	auto qualified_name = qualify_name_globally();
	_names.pop_back();

	return qualified_name;
}

std::string
c4rt2c::ast2_ir_emitter::name_manager::global_name(const c4::ast2::symbol& sym) {
	return fmt::format("q{}S{}E", 1, sym.mangle());
}

c4rt2c::ast2_ir_emitter::function_scope&
c4rt2c::ast2_ir_emitter::last_scope() {
	ASSERT(!_scopes.empty(), "no function scope to return");

	return _scopes.back();
}

llvm::Function*
c4rt2c::ast2_ir_emitter::declare_function(const std::string_view name) {
	if (_let_only) {
		ASSERT(!name.empty(), "function name must not be empty");

		const auto decl = llvm::Function::Create(
			_function_type,
			llvm::Function::ExternalLinkage,
			name,
			_module
		);
		decl->getArg(0)->setName("argv");
		decl->getArg(1)->setName("K");
		_predeclared_functions.emplace(name, decl);

		return decl;
	}

	const auto it = _predeclared_functions.find(std::string{name});
	ASSERT(it != _predeclared_functions.end(), "function not predeclared", name);
	return it->second;
}

llvm::BasicBlock*
c4rt2c::ast2_ir_emitter::define(llvm::Function* fn_decl) {
	ASSERT(fn_decl, "fn_decl must not be null");
	ASSERT(fn_decl->isDeclaration(), "defining a non-declaration");

	return llvm::BasicBlock::Create(_context, "entry", fn_decl);
}

llvm::Function*
c4rt2c::ast2_ir_emitter::resolve_referenced(const c4::ast2::symbol& sym) {
	if (const auto ref = sym.references()) return try_get_function_attribute(ref);

	const auto symname = _name_manager.global_name(sym);
	if (const auto it = _extlib_functions.find(symname);
		it != _extlib_functions.end())
		return it->second;

	const auto fn = declare_function(symname);
	_extlib_functions.emplace(symname, fn);
	return fn;
}
