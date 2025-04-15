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
 * src/c4c/src/ir_emitter/visit_block_args --
 *   Implements the do_visit(block_args&) member function of ir_emitter.
 */

#include <ranges>

#include <c4c/ir_emitter.hxx>
#include <c4c/llvm_value_attribute.hxx>

#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>

#include <libassert/assert.hpp>

void
c4c::ir_emitter::do_visit(const c4::ast2::block_args& obj) {
    const auto formal_args_size = _function_is_closure
                                  ? _active_function->arg_size() - 1
                                  : _active_function->arg_size();
    ASSERT(formal_args_size == obj.size(),
           "defining function with invalid argument size block");

    const auto args = _active_function->args();
    auto arg_begin = args.begin();
    std::size_t i = 0;
    if (_function_is_closure) {
        ++i;
        std::advance(arg_begin, 1);
    }

    for (const auto& [formal_arg, ll_arg] : std::views::zip(obj.args(),
                                                            std::span{arg_begin, args.end()})) {
        ASSERT(ll_arg.getType() == llvm::Type::getInt64Ty(context),
               "trying to name non datum_t type",
               ll_arg.getType()->getTypeID(),
               formal_arg->name(),
               _active_function->getFunctionType()->params(),
               _active_function->getName());
        ll_arg.setName(formal_arg->mangle());
        obj.argument_reference(i++).emplace_attribute<llvm_value_attribute>("value", &ll_arg);
    }
}

