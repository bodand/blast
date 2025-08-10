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

#include <ranges>

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

namespace {
    auto
    load_value_from_symbol(const c4::ast2::symbol* closure_symbol) {
        ASSERT(closure_symbol->references(),
               "symbol captured is not emitted",
               closure_symbol->name(),
               closure_symbol->base_arity());
        auto attr = closure_symbol->references()->get_attribute("ctx_value");
        if (!attr) attr = closure_symbol->references()->get_attribute("value");
        ASSERT(attr,
               "symbol captured is not emitted (refers to something without value attribute)",
               closure_symbol->name(),
               closure_symbol->base_arity(),
               closure_symbol->references());
        const auto val = attr->value<llvm::Value*>();
        DEBUG_ASSERT(val,
                     "symbol's value does not hold llvm::Value*",
                     val);
        return val;
    }

    void
    store_at_index(llvm::IRBuilder<>& builder,
                   llvm::Value* array, std::size_t idx,
                   llvm::Value* val,
                   const llvm::Twine& inst_name = "") {
        const auto ptr_t = llvm::PointerType::get(builder.getContext(), 0);
        const auto index_t = llvm::Type::getInt64Ty(builder.getContext());

        const auto store_ptr = builder.CreateGEP(
            ptr_t,
            array,
            llvm::ConstantInt::get(index_t, idx++),
            inst_name
        );
        builder.CreateStore(val, store_ptr);
    }
}

void
c4c::ir_emitter::initialize_closure_context_storage(llvm::Value* const context_storage,
                                                    const std::span<const c4::ast2::symbol* const> symbols) const {
    for (size_t idx = 0;
         const auto& symbol : symbols) {
        const auto val = load_value_from_symbol(symbol);
        store_at_index(builder, context_storage, idx++, *val,
                       llvm::Twine(_block_name, ".ctx.").concat({symbol->name(), ".addr"}));
    }
}

llvm::Value*
c4c::ir_emitter::create_closure_context(const std::span<const c4::ast2::symbol* const> effective_closure_symbols) {
    const auto ptr_t = llvm::PointerType::get(context, 0);
    const auto size_t = llvm::Type::getInt64Ty(context);
    const auto context_storage =
            builder.CreateAlloca(
                ptr_t,
                llvm::ConstantInt::get(size_t, effective_closure_symbols.size()),
                {_block_name, ".ctx"});

    initialize_closure_context_storage(context_storage, effective_closure_symbols);

    return context_storage;
}

std::vector<const c4::ast2::symbol*>
c4c::ir_emitter::filter_closure_symbols(const c4::ast2::expression& obj) {
    const auto closure_symbols = obj.closure_symbols();
    return closure_symbols
           | std::views::filter([this](const auto& sym) { return !is_skipped_in_context(sym); })
           | std::views::transform([](const auto& sym) { return &sym; })
           | std::ranges::to<std::vector<const c4::ast2::symbol*>>();
}

llvm::Value*
c4c::ir_emitter::create_effective_closure(const c4::ast2::expression& obj) {
    if (!obj.closure()) return nullptr;

    const auto effective_closure_symbols = filter_closure_symbols(obj);
    if (effective_closure_symbols.empty()) return nullptr;

    return create_closure_context(effective_closure_symbols);
}

[[nodiscard]] bool
c4c::ir_emitter::emit_closure_context(const c4::ast2::let_expression& obj) {
    const auto call_ctx = create_effective_closure(obj.value());
    if (!call_ctx) return false;

    obj.emplace_attribute<llvm_value_attribute>("context", call_ctx);
    return true;
}

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
        _block_name = _block_name.substr(0, memory_len);
        return;
    }

    const auto effective_closure = emit_closure_context(obj);

    const auto fn_type = rt_emitter.get_c4_funtype(arity, effective_closure);

    auto linkage = llvm::GlobalValue::ExternalLinkage;
    if (_block_name.contains('$')) linkage = llvm::GlobalValue::PrivateLinkage;
    const auto fn = llvm::Function::Create(fn_type, linkage,
                                           {"_C", _block_name},
                                           module);
    obj.emplace_attribute<llvm_value_attribute>("value", fn);

    enter_function_emission(fn, effective_closure, obj.value(), [this](const c4::ast2::expression& val) {
        // If we need to compute the value, but it is just a block, we just ignore the "pure"
        // semantics of the language and skip evaluating the block expression to get the block
        // object and instead just emit the block as-is.
        // This will result in equivalent behavior: the first call should, in theory, compute and
        // store the block object. After this, including the first call after the initialization
        // step, all references to this symbol should execute the previously stored block object.
        // Instead, we just always execute the body of the block.
        if (std::holds_alternative<c4::ast2::block*>(val.value())) {
            val.accept_skip_self(*this);
            build_return();
            return;
        }

        // We need a global internal variable to hold the computed value.
        const auto datum_t = llvm::Type::getInt64Ty(context);
        const auto undef = llvm::ConstantInt::get(datum_t, gC4_Uninitialized);

        // ReSharper disable once CppDFAMemoryLeak (module param takes ownership)
        const auto value_holder = new llvm::GlobalVariable(module, datum_t,
                                                           false, llvm::GlobalValue::InternalLinkage,
                                                           undef,
                                                           {"_Cv", _block_name});

        val.accept_skip_self(*this);
        const auto ip = builder.saveIP();

        const auto calc_entry_block = &*_active_function->begin();

        const auto load_block = llvm::BasicBlock::Create(context, "value_load", _active_function, calc_entry_block);
        builder.SetInsertPoint(load_block);
        const auto stored_value = builder.CreateLoad(datum_t, value_holder);
        const auto is_defined = builder.CreateICmpEQ(stored_value, undef);

        const auto return_block = llvm::BasicBlock::Create(context, "value_return", _active_function);
        builder.SetInsertPoint(return_block);
        builder.CreateRet(stored_value);

        builder.SetInsertPoint(load_block);
        builder.CreateCondBr(is_defined, calc_entry_block, return_block);

        // builder.SetInsertPoint(calc_entry_block);
        // const auto calced_value = &calc_entry_block->back();
        builder.restoreIP(ip);
        builder.CreateStore(last.value, value_holder);
        builder.CreateRet(last.value);
    });

    _block_name = _block_name.substr(0, memory_len);
}
