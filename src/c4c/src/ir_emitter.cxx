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
 * src/c4c/src/rt_emitter --
 *   
 */

#include <c4c/c4_runtime_emitter.hxx>
#include <c4c/ir_emitter.hxx>
#include <c4c/llvm_value_attribute.hxx>

#include <llvm/Support/Casting.h>

#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>

#include <libassert/assert.hpp>

c4c::ir_emitter::ir_emitter(c4_runtime_emitter& rt_emitter,
                            c4::ast2::ast_context& ast_context,
                            llvm::LLVMContext& context,
                            llvm::Module& module,
                            llvm::IRBuilder<>& builder,
                            std::vector<c4::ast2::undef_symbol> const& promised_symbols)
    : rt_emitter{rt_emitter}
    , ast_context{ast_context}
    , context{context}
    , module{module}
    , builder{builder}
    , promised_symbols(promised_symbols.begin(), promised_symbols.end()) {
    const auto entry_type = llvm::FunctionType::get(llvm::Type::getInt64Ty(context), {}, false);
    entry = llvm::Function::Create(entry_type, llvm::Function::ExternalLinkage, "_c4__entry", module);

    const auto main_body = llvm::BasicBlock::Create(context, "body", entry);
    builder.SetInsertPoint(main_body);
}

void
c4c::ir_emitter::finalize() {
    _finalized = true;
    build_return();
    generate_cleanup(&entry->back(), _need_cleanup);
}

c4c::ir_emitter::~ir_emitter() noexcept {
    ASSERT(_finalized, "ir_emitter must be finalized: call finalize on it before it dies");
}

bool
c4c::ir_emitter::skip_in_context(const c4::ast2::symbol& sym) {
    const auto fn = lookup(sym);
    if (fn == nullptr) return true;
    return isa<llvm::Function>(fn);
}

llvm::Value*
c4c::ir_emitter::try_materialize_promise(const std::string_view sym) {
    const auto promised_sym_it = std::ranges::find_if(promised_symbols, [sym](const auto undef_sym) {
        return undef_sym.mangle() == sym;
    });
    if (promised_sym_it == promised_symbols.end())
        return nullptr;

    const auto promised_sym = *promised_sym_it;
    promised_symbols.erase(promised_sym_it);

    const auto promised_fn_type = rt_emitter.get_c4_funtype(promised_sym.arity());
    const auto promised_fn = llvm::Function::Create(promised_fn_type, llvm::Function::ExternalLinkage,
                                                    {"_C", promised_sym.mangle()},
                                                    module);

    _loaded_promised_symbols[std::string(sym)] = promised_fn;

    return promised_fn;
}

llvm::Value*
c4c::ir_emitter::lookup(const c4::ast2::symbol& sym) {
    if (const auto ref = sym.references()) {
        const auto callee_ref = ref->attribute_value<llvm::Value*>("value");
        ASSERT(callee_ref, "referenced symbol must have a value attribute to the callee",
               sym.mangle(),
               callee_ref);
        return *callee_ref;
    }

    if (const auto known_it = _loaded_promised_symbols.find(sym.mangle());
        known_it != _loaded_promised_symbols.end()) {
        return known_it->second;
    }

    return try_materialize_promise(sym.mangle());
}

llvm::Value*
c4c::ir_emitter::lookup_symbol(const std::string_view sym) {
    if (const auto known_it = _loaded_promised_symbols.find(std::string(sym));
        known_it != _loaded_promised_symbols.end()) {
        return known_it->second;
    }

    return try_materialize_promise(sym);
}

void
c4c::ir_emitter::build_return() {
    if (const auto it = std::ranges::find(_need_cleanup, last.value);
        it != _need_cleanup.end()) {
        // don't clean up stuff we are returning
        _need_cleanup.erase(it);
    }
    builder.CreateRet(last.value);
}

void
c4c::ir_emitter::generate_cleanup(llvm::BasicBlock* fn_body, const std::span<llvm::Value* const> cleanup) const {
    if (cleanup.empty()) return;

    const auto block_sz = static_cast<long long>(fn_body->size());
    const auto cleanup_block = fn_body->splitBasicBlock(std::next(fn_body->begin(), block_sz - 1), "cleanup");
    builder.SetInsertPoint(cleanup_block->begin());
    for (const auto& to_clean : cleanup) {
        rt_emitter.emit_rt_call(c4c::c4rt_symbol::DatumFree, builder, to_clean);
    }
}

c4c::scoped_memento
c4c::ir_emitter::save_state() {
    return {
        std::unique_ptr<ir_emitter_memento>(new ir_emitter_memento(
            _active_function,
            _function_is_closure,
            _need_cleanup,
            builder.saveIP())),
        *this
    };
}

llvm::BasicBlock*
c4c::ir_emitter::build_bblock(const std::string_view name) const {
    const auto bb = llvm::BasicBlock::Create(context, name, _active_function);
    builder.SetInsertPoint(bb);
    return bb;
}
