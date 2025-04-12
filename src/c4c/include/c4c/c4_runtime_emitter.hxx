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
 * Originally created: 2025-03-03.
 *
 * src/c4c/include/c4c/c4_runtime_emitter --
 *   Helper to emit LLVM IR to calls into the C4 runtime.
 */
#ifndef C4C_C4_RUNTIME_EMITTER_HXX
#define C4C_C4_RUNTIME_EMITTER_HXX

#include <llvm/IR/LLVMContext.h>

#include <unordered_map>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Value.h>

namespace c4c {
    enum class c4rt_symbol {
        DatumFromInt32,
        DatumFromInt64,
        DatumFromDouble,
        DatumFromString,
        DatumFromProc,
        DatumFree,
        DatumGetInt32,
        DatumGetInt64,
        DatumGetDouble,
        DatumGetString,
        DatumCoerceInt32,
        DatumCoerceInt64,
        DatumCoerceDouble,
        DatumCoerceString,
    };

    struct c4_runtime_emitter {
        c4_runtime_emitter(llvm::LLVMContext& context, llvm::Module& module);

        c4_runtime_emitter(const c4_runtime_emitter& other) = delete;

        c4_runtime_emitter(c4_runtime_emitter&& other) noexcept = delete;

        c4_runtime_emitter&
        operator=(const c4_runtime_emitter& other) = delete;

        c4_runtime_emitter&
        operator=(c4_runtime_emitter&& other) noexcept = delete;

        llvm::Value*
        emit_rt_call(c4rt_symbol sym,
                     llvm::IRBuilder<>& builder,
                     llvm::ArrayRef<llvm::Value*> args);

        [[nodiscard]] llvm::FunctionType*
        get_c4_funtype(unsigned arity, bool context = false) const;

    private:
        llvm::LLVMContext& _context;
        std::unordered_map<c4rt_symbol, llvm::Function*> _symbol_map;
    };
}

#endif
