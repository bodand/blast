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
 * src/c4c/src/rt_emitter/visit_fn_call --
 *   Implements the do_visit(fn_call&) member function of ir_emitter.
 */

#include <c4c/ir_emitter.hxx>

#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>

#include <libassert/assert.hpp>

void
c4c::ir_emitter::do_visit(const c4::ast2::fn_call& obj) {
    const auto callee = lookup(obj.sym());
    ASSERT(callee, "called function cannot be found",
           obj.sym().name(),
           promised_symbols,
           _loaded_promised_symbols);

    if (callee->getType()->isIntegerTy()) {
        last.set_expr(callee);
        return;
    }

    // has arguments, so callee must be a function type
    ASSERT(callee->getType()->isPointerTy(),
           "fn called with parameters does not have function (pointer) type",
           callee->getName(),
           callee->getType()->getTypeID());
    const auto fn = cast<llvm::Function>(callee);
    ASSERT(fn);

    if (const auto fn_type = fn->getFunctionType();
        fn_type->getNumParams() > 0
        && fn_type->getParamType(0)->isPointerTy()) {
        const auto ref = obj.sym().references();
        const auto context_ref = ref->attribute_value<llvm::Value*>("context");
        ASSERT(context_ref, "referenced symbol must have a context attribute (0th parameter type is ptr)",
               obj.sym().mangle(),
               context_ref);
        const auto context = *context_ref;

        std::vector<llvm::Value*> args(obj.args().size() + 1);
        args[0] = context;
        std::transform(obj.args().begin(), obj.args().end(),
                       next(args.begin()),
                       [this](const c4::ast2::expression* const& arg) {
                           arg->accept_skip_self(*this);
                           return this->last.value;
                       });

        const auto call = builder.CreateCall(fn_type, callee, args);
        last.set_expr(call);

        return;
    }

    std::vector<llvm::Value*> args(obj.args().size());
    std::transform(obj.args().begin(), obj.args().end(), args.begin(),
                   [this](const c4::ast2::expression* const& arg) {
                       arg->accept_skip_self(*this);
                       return this->last.value;
                   });

    const auto call = builder.CreateCall(fn->getFunctionType(), callee, args);
    last.set_expr(call);
}
