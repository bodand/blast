/* blAST project
 *
 * Copyright (c) 2025 András Bodor <bodand@pm.me>
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
 * Originally created: 2025-08-08.
 *
 * src/c4c/src/ffm_ir_emitter --
 *   
 */

#include <string>
#include <string_view>
#include <tuple>
#include <vector>
#include <array>
#include <span>

#include <c4/ffm.hxx>
#include <c4c/llvm_value_attribute.hxx>

#include <llvm/Support/Casting.h>

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>

#include <libassert/assert.hpp>

#include <c4c/ffm_ir_emitter.hxx>

namespace {
    llvm::Function*
    try_get_function_attribute(const c4::ast2::tags::attributable* ref) {
        const auto attr = ref->attribute_value<llvm::Function*>("function");
        if (!attr) return nullptr;

        return *attr;
    }

    llvm::Type*
    try_get_type_attribute(const c4::ast2::tags::attributable* ref) {
        const auto attr = ref->attribute_value<llvm::Type*>("type");
        if (!attr) return nullptr;

        return *attr;
    }
}

c4c::ffm_ir_emitter::ffm_ir_emitter(llvm::LLVMContext& context, llvm::Module& module,
                                    llvm::IRBuilder<llvm::ConstantFolder, llvm::IRBuilderDefaultInserter>& builder,
                                    llvm::FunctionPassManager& pass_manager,
                                    llvm::FunctionAnalysisManager& fna_manager)
    : entry{nullptr}
    , _pass_manager{pass_manager}
    , _fna_manager{fna_manager}
    , _context{context}
    , _module{module}
    , _builder{builder}
    , _c4rt_datum_type{llvm::Type::getInt64Ty(context)}
    , _c4rt_package_type{llvm::PointerType::get(context, 0)}
    , _rt_emitter{_module, &_builder} { }

void
c4c::ffm_ir_emitter::do_visit(const c4::ffm::block_argument& obj) {
    const auto llvm_arg = obj.attribute_value<llvm::Value*>("value");
    ASSERT(llvm_arg, "block argument must have a value attribute", obj.name());
    _current_args.push_back(*llvm_arg);
}

void
c4c::ffm_ir_emitter::do_visit(const c4::ffm::context_type& obj) {
    const auto types = std::vector(obj.fields().size(), _c4rt_package_type);

    const auto ctx_type = llvm::StructType::create(_context, types, obj.name());
    obj.emplace_attribute<llvm_type_attribute>("type", ctx_type);
}

void
c4c::ffm_ir_emitter::do_visit(const c4::ffm::context_object& obj) {
    const auto ctx_type = try_get_type_attribute(obj.ctx_type());
    ASSERT(ctx_type, "constructed context object must have known type layout", obj.ctx_type()->name());

    const auto local_var = _builder.CreateAlloca(ctx_type);
    for (std::size_t i = 0;
         const auto field : obj.args()) {
        const auto scope = call_stack(_active_call, _current_args);
        field->accept(*this);
        const auto field_val = _current_args.back();
        ASSERT(field_val, "context object must refer to proper llvm::Value",
               obj.ctx_type()->name());
        const auto member_ptr = _builder.CreateStructGEP(ctx_type, local_var, i++);
        _builder.CreateStore(field_val, member_ptr);
    }
    _current_args.push_back(local_var);
}

void
c4c::ffm_ir_emitter::do_visit(const c4::ffm::context_access& obj) {
    const auto name = obj.name();
    const auto llvm_type = try_get_type_attribute(obj.ctx_type());
    ASSERT(llvm_type, "context type must have a type attribute", obj.ctx_type()->name());

    obj.arg()->accept(*this);
    const auto arg_val = _current_args.back();
    _current_args.pop_back();

    const auto fields = obj.ctx_type()->fields();
    const auto field_it = std::ranges::find_if(fields, [&](const auto& field) {
        return field->name() == name;
    });
    ASSERT(field_it != fields.end(), "context object must have a field with the accessed name",
           obj.ctx_type()->name(),
           obj.name());
    const auto field_idx = std::distance(fields.begin(), field_it);
    const auto field_type = llvm_type->getStructElementType(field_idx);
    const auto field_ptr = _builder.CreateStructGEP(llvm_type, arg_val, field_idx);
    const auto field_val = _builder.CreateLoad(field_type, field_ptr);
    push_value(field_val);
}

void
c4c::ffm_ir_emitter::do_visit(const c4::ffm::root_expression& obj) {
    obj.accept_skip_self(*this);
}

void
c4c::ffm_ir_emitter::do_visit(const c4::ffm::value_expression& obj) {
    obj.accept_skip_self(*this);
}

void
c4c::ffm_ir_emitter::do_visit(const c4::ffm::function_call& obj) {
    const auto stack = call_stack(_active_call, _current_args);
    for (const auto& argument : obj.arguments()) argument->accept(*this);

    const auto decl = obj.function();
    DEBUG_ASSERT(decl, "function call must have a function");
    const auto llvm_decl = try_get_function_attribute(decl);
    ASSERT(llvm_decl, "function must be declared in ffm before it is called");

    if (obj.packed()) {
        if (obj.function()->closure()) {
            const auto pkg = _rt_emitter.local_package();
            const auto ctx = _current_args.front();
            _rt_emitter.emit_package_init_from_closure(pkg,
                                                       llvm_decl,
                                                       ctx,
                                                       std::span(std::next(_current_args.begin()),
                                                                 _current_args.end()));
            stack.pop();
            push_value(pkg);
        }
        else {
            const auto pkg = _rt_emitter.local_package();
            _rt_emitter.emit_package_init_from_function(pkg, llvm_decl, _current_args);

            stack.pop();
            push_value(pkg);
        }
    }
    else {
        const auto fn_type = llvm_decl->getFunctionType();
        const auto call = _builder.CreateCall(fn_type, llvm_decl, _current_args);

        stack.pop();
        push_value(call);
    }
}

void
c4c::ffm_ir_emitter::do_visit(const c4::ffm::dynamic_call& obj) {
    const auto stack = call_stack(_active_call, _current_args);
    for (const auto& argument : obj.arguments()) argument->accept(*this);

    const auto callee = obj.callee();
    callee->accept(*this);

    const auto callee_value = _current_args.back();
    _current_args.pop_back();

    const auto callee_datum = _rt_emitter.emit_unpack(callee_value);
    if (_in_unpack) {
        // in unpack, call is directly evaluated in root block call eagerly
        _in_unpack = false;

        const auto result_datum = _rt_emitter.emit_datum_evaluate(callee_datum, _current_args);
        stack.pop();

        return push_value(result_datum);
    }

    const auto packaged = _rt_emitter.local_package();
    _rt_emitter.emit_package_init_from_dynamic(packaged, callee_datum, _current_args);
    stack.pop();

    push_value(packaged);
}

void
c4c::ffm_ir_emitter::do_visit(const c4::ffm::function_declaration& obj) {
    const auto fn_type = build_type_for(obj);
    const auto linkage = obj.known()
                         ? llvm::Function::InternalLinkage
                         : llvm::Function::ExternalLinkage;
    const auto fn = llvm::Function::Create(fn_type,
                                           linkage,
                                           obj.name(),
                                           _module);

    if (const auto ctx = obj.ctx_type()) ctx->accept(*this);

    for (std::size_t i = 0;
         auto& arg : std::span(fn->arg_begin(), fn->arg_end())) {
        const auto ffm_arg = obj.argument(i++);
        arg.setName(ffm_arg->name());
        ffm_arg->emplace_attribute<llvm_value_attribute>("value", &arg);
    }
    obj.emplace_attribute<llvm_function_attribute>("function", fn);
}

void
c4c::ffm_ir_emitter::do_visit(const c4::ffm::function_definition& obj) {
    const auto decl = obj.decl();
    DEBUG_ASSERT(decl, "declaration of definition must be set", obj.name());
    const auto llvm_decl = try_get_function_attribute(decl);
    ASSERT(llvm_decl, "function must be declared in ffm before it is defined", obj.name());

    const auto ip = _builder.saveAndClearIP();
    _current_function = llvm_decl;
    std::ignore = build_bblock("body");

    for (std::size_t i = 0;
         const auto& expression : obj.body()) {
        _returned_value = i == obj.body().size() - 1;
        expression->accept(*this);
        ++i;
    }
    _returned_value = false;
    _pass_manager.run(*_current_function, _fna_manager);

    _builder.restoreIP(ip);
}

namespace {
    struct literal_visitor final {
        c4rt2c::c4_rt2_emitter& _rt_emitter;

        llvm::Value*
        operator()(const std::int32_t x) const {
            return _rt_emitter.encode_datum_int32(x);
        }

        llvm::Value*
        operator()(const std::int64_t x) const {
            return _rt_emitter.encode_datum_int64(x);
        }

        llvm::Value*
        operator()(const double x) const {
            return _rt_emitter.encode_datum(x);
        }

        llvm::Value*
        operator()(const std::string_view x) const {
            return _rt_emitter.encode_datum_string(x);
        }
    };
}

void
c4c::ffm_ir_emitter::do_visit(const c4::ffm::literal& obj) {
    const auto llvm_lit = build_literal(obj);
    push_value(llvm_lit);
}

void
c4c::ffm_ir_emitter::do_visit(const c4::ffm::block_literal& obj) {
    const auto scope = call_stack(_active_call, _current_args);
    llvm::Value* ctx_obj = nullptr;
    llvm::Value* ctx_obj_sz = nullptr;
    if (const auto ctx = obj.context()) {
        ctx->accept(*this);
        ctx_obj = _current_args.back();
        _current_args.pop_back();

        const auto ctx_type = try_get_type_attribute(ctx->ctx_type());
        DEBUG_ASSERT(ctx_type, "context type must have a type attribute", ctx->ctx_type()->name());
        const auto ctx_sz = _module.getDataLayout().getTypeAllocSize(ctx_type);
        ctx_obj_sz = llvm::ConstantInt::get(_context, llvm::APInt(64, ctx_sz));
    }

    const auto base_arity = obj.function()->base_arity();
    const auto arity_val = llvm::ConstantInt::get(_context, llvm::APInt(16, base_arity));

    const auto fn = try_get_function_attribute(obj.function());
    ASSERT(fn, "block literal must have a function attribute",
           obj.function()->name());

    llvm::Value* args = llvm::ConstantPointerNull::get(llvm::PointerType::get(_context, 0));
    llvm::Value* args_sz = llvm::ConstantInt::get(_context, llvm::APInt(64, 0));
    if (!_current_args.empty()) {
        args_sz = llvm::ConstantInt::get(_context, llvm::APInt(64, _current_args.size()));
        if (_current_args.size() == 1) {
            args = _current_args.front();
        }
        else {
            args = _rt_emitter.local_package_array(_current_args.size());
            for (size_t i = 0;
                 const auto arg : _current_args) {
                const auto arg_ptr = _builder.CreateInBoundsGEP(
                    args->getType(),
                    args,
                    {
                        llvm::ConstantInt::get(_context, llvm::APInt(64, i++))
                    });
                _builder.CreateMemCpy(arg_ptr, llvm::Align(8), arg, llvm::Align(8), 4 * 64 / 8);
            }
        }
    }

    const auto datum = ctx_obj
                       ? _rt_emitter.emit_datum_from_closure(fn,
                                                             arity_val,
                                                             ctx_obj,
                                                             ctx_obj_sz,
                                                             args,
                                                             args_sz)
                       : _rt_emitter.emit_datum_from_function(fn,
                                                              arity_val,
                                                              args,
                                                              args_sz);
    scope.pop();
    if (obj.packed()) {
        const auto pkg = _rt_emitter.local_package();
        _rt_emitter.emit_package_init_from_result(pkg, datum);
        push_value(pkg);
    }
    else {
        push_value(datum);
    }
}

void
c4c::ffm_ir_emitter::do_visit(const c4::ffm::local& obj) {
    const auto scope = call_stack(_active_call, _current_args);
    obj.value()->accept(*this);
    ASSERT(_current_args.size() == 1, "local must be set with one value");
    auto value = _current_args.back();
    value->setName(obj.name());
    obj.emplace_attribute<llvm_value_attribute>("value", value);
}

void
c4c::ffm_ir_emitter::do_visit(const c4::ffm::local_ref& obj) {
    const auto val = obj.ref()->attribute_value<llvm::Value*>("value");
    ASSERT(val, "referenced symbol must have a value attribute to the callee",
           obj.ref()->name(),
           val);
    push_value(*val);
}

void
c4c::ffm_ir_emitter::do_visit(const c4::ffm::unpack& obj) try {
    const auto scope = call_stack(_active_call, _current_args);
    _in_unpack = true;
    obj.expr()->accept(*this);
    ASSERT(_current_args.size() == 1, "unpack must be called with one value");
    const auto unpackee = _current_args.back();
    scope.pop();

    if (!_in_unpack) return push_value(unpackee);

    const auto val = _rt_emitter.emit_unpack(unpackee);
    push_value(val);
    _in_unpack = false;
}
catch (...) {
    _in_unpack = false;
    throw;
}

void
c4c::ffm_ir_emitter::do_visit(const c4::ffm::function& obj) {
    obj.accept_skip_self(*this);
}

void
c4c::ffm_ir_emitter::push_value(llvm::Value* value) {
    if (_active_call) return _current_args.push_back(value);
    if (_returned_value) {
        _builder.CreateRet(value);
        return;
    }
}

llvm::Value*
c4c::ffm_ir_emitter::build_literal(const c4::ffm::literal& lit) {
    literal_visitor visitor{_rt_emitter};
    const auto lit_val = std::visit(visitor, lit.value());
    if (!lit.packed()) return lit_val;

    const auto pkg = _rt_emitter.local_package();
    _rt_emitter.emit_package_init_from_result(pkg, lit_val);
    return pkg;
}

llvm::FunctionType*
c4c::ffm_ir_emitter::build_type_for(const c4::ffm::function_declaration& decl) const {
    return build_type_with_arity(decl.effective_arity());
}

llvm::FunctionType*
c4c::ffm_ir_emitter::build_type_with_arity(const unsigned arity) const {
    std::vector<llvm::Type*> args(arity);
    std::ranges::generate(args, [&] { return _c4rt_package_type; });
    return llvm::FunctionType::get(_c4rt_datum_type, args, false);
}

llvm::BasicBlock*
c4c::ffm_ir_emitter::build_bblock(const std::string_view name) const {
    const auto bb = llvm::BasicBlock::Create(_context, name, _current_function);
    _builder.SetInsertPoint(bb);
    return bb;
}
