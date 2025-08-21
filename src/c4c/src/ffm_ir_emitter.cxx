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
}

c4c::ffm_ir_emitter::ffm_ir_emitter(llvm::LLVMContext& context,
                                    llvm::Module& module,
                                    llvm::IRBuilder<>& builder)
    : entry{nullptr}
    , _context{context}
    , _module{module}
    , _builder{builder}
    , _c4rt_datum_type{llvm::Type::getInt64Ty(context)}
    , _c4rt_package_type{llvm::Type::getInt64Ty(context)}
    , _rt_emitter{_module, &_builder} { }

void
c4c::ffm_ir_emitter::do_visit(const c4::ffm::block_argument& obj) { }

void
c4c::ffm_ir_emitter::do_visit(const c4::ffm::context_type& obj) { }

void
c4c::ffm_ir_emitter::do_visit(const c4::ffm::context_object& obj) { }

void
c4c::ffm_ir_emitter::do_visit(const c4::ffm::context_access& obj) { }

void
c4c::ffm_ir_emitter::do_visit(const c4::ffm::root_expression& obj) {
    obj.accept_skip_self(*this);
}

void
c4c::ffm_ir_emitter::do_visit(const c4::ffm::value_expression& obj) { }

void
c4c::ffm_ir_emitter::do_visit(const c4::ffm::function_call& obj) { }

void
c4c::ffm_ir_emitter::do_visit(const c4::ffm::dynamic_call& obj) { }

void
c4c::ffm_ir_emitter::do_visit(const c4::ffm::function_declaration& obj) {
    const auto fn_type = build_type_for(obj);
    const auto fn = llvm::Function::Create(fn_type,
                                           llvm::Function::InternalLinkage,
                                           obj.name(),
                                           _module);
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
    const auto fn_body = build_bblock("body");

    for (const auto& expression : obj.body()) {
        expression->accept(*this);
    }

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
    DEBUG_ASSERT(!obj.packed(), "sorry, not implemented", obj.value());

    literal_visitor visitor{_rt_emitter};
    _builder.CreateRet(std::visit(visitor, obj.value()));
}

void
c4c::ffm_ir_emitter::do_visit(const c4::ffm::block_literal& obj) { }

void
c4c::ffm_ir_emitter::do_visit(const c4::ffm::local& obj) { }

void
c4c::ffm_ir_emitter::do_visit(const c4::ffm::local_ref& obj) { }

void
c4c::ffm_ir_emitter::do_visit(const c4::ffm::unpack& obj) { }

void
c4c::ffm_ir_emitter::do_visit(const c4::ffm::function& obj) {
    obj.accept_skip_self(*this);
}

llvm::FunctionType*
c4c::ffm_ir_emitter::build_type_for(const c4::ffm::function_declaration& decl) {
    DEBUG_ASSERT(!decl.closure(), "closure typing not yet implemented");

    const auto basic_args_sz = decl.base_arity();
    std::vector<llvm::Type*> args(basic_args_sz);
    std::ranges::generate(args, [&] { return _c4rt_package_type; });

    return llvm::FunctionType::get(_c4rt_datum_type, args, false);
}

llvm::BasicBlock*
c4c::ffm_ir_emitter::build_bblock(const std::string_view name) const {
    const auto bb = llvm::BasicBlock::Create(_context, name, _current_function);
    _builder.SetInsertPoint(bb);
    return bb;
}
