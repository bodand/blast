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
 * src/c4rt2/include/c4rt2c/c4rt_emitter --
 *   Emitter for generating LLVM IR for calling into
 */

#ifndef BLAST_C4RT_EMITTER_HXX
#define BLAST_C4RT_EMITTER_HXX

#include <cstdint>
#include <string_view>

namespace llvm {
    class Value;
    class ConstantFolder;
    class IRBuilderDefaultInserter;
    template<typename FolderTy, typename InserterTy>
    class IRBuilder;
}

namespace c4rt2c {
    struct c4_rt2_emitter {
        c4_rt2_emitter(llvm::Module& module, llvm::IRBuilder<>* builder);

        [[nodiscard]] llvm::Value*
        emit_datum_from_static_ptr(llvm::Value* type, llvm::Value* ptr) const;

        [[nodiscard]] llvm::Value*
        encode_datum(double d) const;

        [[nodiscard]] llvm::Value*
        encode_datum_int32(std::int32_t i) const;

        [[nodiscard]] llvm::Value*
        encode_datum_int64(std::int64_t i) const;

        [[nodiscard]] llvm::Value*
        encode_datum_string(std::string_view i) const;

    private:
        llvm::Type* _c4rt_datum_type;
        llvm::Type* _c4rt_package_type;

        llvm::IRBuilder<llvm::ConstantFolder, llvm::IRBuilderDefaultInserter>* _builder;
        llvm::Module& _module;
    };
}

#endif
