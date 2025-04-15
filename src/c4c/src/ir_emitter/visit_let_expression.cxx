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
 * src/c4c/src/ir_emitter/visit_let_expression --
 *   
 */

#include <c4c/global_constant_emitter.hxx>
#include <c4c/c4_runtime_emitter.hxx>
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
c4c::ir_emitter::do_visit(const c4::ast2::let_expression& obj) {
    const auto obj_name = obj.mangled_name();
    const auto arity = obj.symbol_arity();
    const auto memory_len = _block_name.size();
    if (_block_name.empty()) {
        _block_name = obj_name;
    }
    else {
        _block_name = fmt::format("{}${}", _block_name, obj_name);
    }

    if (obj.value().const_evaluable()) {
        global_constant_emitter constant_emitter(context, module, builder);
        obj.value().accept_skip_self(constant_emitter);
        last.set_constant(constant_emitter.get_loaded_global(_block_name));
        obj.emplace_attribute<llvm_value_attribute>("value", last.value);
        // _known_symbols[_block_name] = last.value;
        _block_name = _block_name.substr(0, memory_len);
        return;
    }

    // WARNING: HORRID HACK: USING WHILE AS IF TO ALLOW BREAKING IT IN THE
    //   MIDDLE, REFACTOR LOGIC INTO FN
    bool effective_closure = obj.value().closure();
    while (effective_closure) {
        const auto datum_t = llvm::Type::getInt64Ty(context);
        const auto index_t = llvm::Type::getInt64Ty(context);

        std::vector<const c4::ast2::symbol*> effective_closure_symbols;
        effective_closure_symbols.reserve(obj.value().closure_symbols().size());
        std::ranges::transform(obj.value().closure_symbols(),
                               std::back_inserter(effective_closure_symbols),
                               [](const auto& sym) { return &sym; });
        std::erase_if(effective_closure_symbols,
                      [this](const auto& sym) mutable { return skip_in_context(*sym); });
        if (effective_closure_symbols.empty()) {
            effective_closure = false;
            break;
        }

        const auto call_ctx =
                builder.CreateAlloca(
                    datum_t,
                    llvm::ConstantInt::get(datum_t, effective_closure_symbols.size()),
                    {_block_name, ".ctx"});

        obj.emplace_attribute<llvm_value_attribute>("context", call_ctx);

        size_t idx = 0;
        for (const auto& closure_symbol : effective_closure_symbols) {
            const auto store_ptr = builder.CreateGEP(
                datum_t,
                call_ctx,
                llvm::ConstantInt::get(index_t, idx++),
                llvm::Twine(_block_name, ".ctx.").concat({closure_symbol->name(), ".addr"})
            );

            ASSERT(closure_symbol->references(),
                   "symbol captured is not emitted",
                   closure_symbol->name(),
                   closure_symbol->arity());
            const auto attr = closure_symbol->references()->get_attribute("value");
            ASSERT(attr,
                   "symbol captured is not emitted (refers to something without value attribute)",
                   closure_symbol->name(),
                   closure_symbol->arity(),
                   closure_symbol->references());
            const auto val = attr->value<llvm::Value*>();
            DEBUG_ASSERT(val,
                         "symbol's value does not hold llvm::Value*",
                         val);
            builder.CreateStore(*val, store_ptr);
        }
        break;
    }

    const auto fn_type = rt_emitter.get_c4_funtype(arity, effective_closure);
    const auto fn = llvm::Function::Create(fn_type, llvm::GlobalValue::ExternalLinkage,
                                           {"_C", _block_name},
                                           module);
    obj.emplace_attribute<llvm_value_attribute>("value", fn);

    enter_function_emission(fn, effective_closure, obj.value(), [this](const auto& val) {
        val.accept_skip_self(*this);
        build_return();
    });

    _block_name = _block_name.substr(0, memory_len);
}
