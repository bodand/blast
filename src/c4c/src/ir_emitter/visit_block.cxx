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
 * Originally created: 2025-04-04.
 *
 * src/c4c/src/ir_emitter/visit_block --
 *   
 */

#include <c4c/ir_emitter.hxx>
#include <c4c/llvm_value_attribute.hxx>

#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>

#include <libassert/assert.hpp>

#include <algorithm>
#include <ranges>

void
c4c::ir_emitter::do_visit(const c4::ast2::block& obj) {
    if (obj.args())
        obj.args()->accept(*this);
    if (obj.requires_context()) {
        // if name is not empty, the ctx contained only globally known things
        // and is not actually the context parameter
        if (_active_function->arg_size() > 0
            && _active_function->arg_begin()->getName().empty())
            _active_function->arg_begin()->setName("ctx");
    }
    emit_fn_body_from_block(obj);
}

void
c4c::ir_emitter::emit_fn_body_from_block(const c4::ast2::block& obj) {
    scope_override_fixer fixer;
    if (obj.requires_context() && _active_function->arg_size() > 0) {
        emit_context_expansion(fixer, obj);
    }
    for (const auto& expr : obj.expressions()) {
        expr->accept_skip_self(*this);
    }
    fixer();
}

void
c4c::ir_emitter::emit_context_expansion(scope_override_fixer& fixer, const c4::ast2::block& obj) {
    const auto body_bb = builder.GetInsertBlock();
    const auto ip = builder.saveIP();

    const auto block = llvm::BasicBlock::Create(context, "ctx_exp", _active_function, body_bb);
    builder.SetInsertPoint(block);

    const auto ptr_t = llvm::PointerType::get(context, 0);
    const auto gep_index_t = llvm::Type::getInt64Ty(context);
    const auto ctx_obj = _active_function->getArg(0);
    if (!ctx_obj->getType()->isPointerTy()) {
        builder.CreateBr(body_bb);
        builder.restoreIP(ip);
        return;
    }

    const auto datum_t = llvm::Type::getInt64Ty(context);
    const auto arg_loader_type = llvm::FunctionType::get(datum_t, {llvm::PointerType::get(context, 0)}, false);
    const auto arg_loader = module.getFunction("_c4__arg_loader");
    ASSERT(arg_loader, "arg loader function not found");

    for (size_t idx = 0;
         auto& sym : obj.effective_context_symbols()) {
        if (is_skipped_in_context(sym)) continue;

        const auto ctx_param_ptr = builder.CreateGEP(ptr_t, ctx_obj,
                                                     llvm::ConstantInt::get(gep_index_t, idx++),
                                                     {sym.name(), ".addr"});
        const auto ctx_param = builder.CreateLoad(ptr_t, ctx_param_ptr, sym.name());

        const auto state = save_state();
        const auto eval_block = llvm::BasicBlock::Create(context, {sym.mangle(), ".eval"});
        _orphan_blocks.push_back(eval_block);
        builder.SetInsertPoint(eval_block);

        builder.CreateCall(arg_loader_type, arg_loader, {ctx_param}, sym.mangle());

        if (const auto ref = sym.references()) fixer.add_symbol(ref, eval_block);
    }

    builder.CreateBr(body_bb);
    builder.restoreIP(ip);
}


void
c4c::ir_emitter::scope_override_fixer::add_symbol(c4::ast2::tags::referable* sym, llvm::Value* new_value) {
    auto old = sym->emplace_attribute<llvm_value_attribute>("value", new_value);
    _overridden.emplace_back(sym, std::move(old));
}

void
c4c::ir_emitter::scope_override_fixer::operator()() {
    std::ranges::for_each(std::ranges::reverse_view(_overridden), [](const scope_override& fixee) {
        fixee.symbol->emplace_attribute<llvm_value_attribute>("value", *fixee.old->value<llvm::Value*>());
    });
}
