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
 * src/c4c/src/ir_emitter/visit_string_literal --
 *   
 */

#include <c4c/c4_runtime_emitter.hxx>
#include <c4c/global_constant_emitter.hxx>
#include <c4c/ir_emitter.hxx>
#include <c4c/llvm_value_attribute.hxx>

#include <c4rt/datum.h>

#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>

#include <libassert/assert.hpp>

void
c4c::ir_emitter::do_visit(const c4::ast2::string_literal& obj) {
    if (obj.const_evaluable()) {
        global_constant_emitter constant_emitter(context, module, builder);
        obj.accept(constant_emitter);
        last.set_constant(constant_emitter.get_loaded_global(_block_name));
        obj.emplace_attribute<llvm_value_attribute>("value", last.value);
        return;
    }
    const auto global = builder.CreateGlobalStringPtr(obj.value(), {_block_name, "_sl"}, 0, &module);
    const auto value = rt_emitter.emit_rt_call(c4rt_symbol::DatumFromString, builder, global);
    last.set_expr(value);
    obj.emplace_attribute<llvm_value_attribute>("value", last.value);
    _need_cleanup.push_back(value);
}

