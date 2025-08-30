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
#include <span>
#include <array>

namespace llvm {
    class Value;
    class Type;
    class StructType;
    class Module;
    class Function;
    class FunctionCallee;
    class ConstantFolder;
    class IRBuilderDefaultInserter;
    template<typename FolderTy, typename InserterTy>
    class IRBuilder;
}

namespace c4rt2c {
    struct c4_rt2_emitter final {
        c4_rt2_emitter(llvm::Module& module, llvm::IRBuilder<llvm::ConstantFolder, llvm::IRBuilderDefaultInserter>* builder);

        [[nodiscard]] llvm::Value*
        emit_datum_from_static_ptr(llvm::Value* type, llvm::Value* ptr) const;

        [[nodiscard]] llvm::Value*
        emit_datum_from_function(llvm::Value* func,
                                 llvm::Value* arity,
                                 llvm::Value* fn_data, llvm::Value* fn_data_sz) const;

        [[nodiscard]] llvm::Value*
        emit_datum_from_closure(llvm::Value* func,
                                llvm::Value* arity,
                                llvm::Value* ctx,
                                llvm::Value* ctx_sz, llvm::Value* fn_data, llvm::Value* fn_data_sz) const;

        [[nodiscard]] llvm::Value*
        emit_datum_evaluate(llvm::Value* datum, std::span<llvm::Value*> args) const;

        void
        emit_package_init(llvm::Value* ptr) const;

        void
        emit_package_init_from_result(llvm::Value* ptr, llvm::Value* datum) const;

        void
        emit_package_init_from_function(llvm::Value* pkg,
                                        llvm::Function* function,
                                        const std::vector<llvm::Value*>& vector) const;

        void
        emit_package_init_from_closure(llvm::Value* pkg,
                                       llvm::Function* function,
                                       llvm::Value* ctx,
                                       std::span<llvm::Value*> vector) const;

        llvm::Value*
        emit_unpack(llvm::Value* value) const;

        [[nodiscard]] llvm::Value*
        local_package() const;

        [[nodiscard]] llvm::Value*
        local_package_array(size_t n) const;

        [[nodiscard]] llvm::Value*
        local_package_array_uninit(size_t n) const;

        [[nodiscard]] llvm::Value*
        encode_datum(double d) const;

        [[nodiscard]] llvm::Value*
        encode_datum_int32(std::int32_t i) const;

        [[nodiscard]] llvm::Value*
        encode_datum_int64(std::int64_t i) const;

        [[nodiscard]] llvm::Value*
        encode_datum_string(std::string_view i) const;

    private:
        void
        set_package_version(llvm::Value* pkg) const;

        template<class... Args>
        void
        declare_rt_function(std::string_view name, llvm::Type* ret_type, Args&&... arg_types) const {
            std::array<llvm::Type*, sizeof...(Args)> args{std::forward<Args>(arg_types)...};
            declare_rt_function_impl(name, ret_type, args);
        }

        void
        declare_rt_function_impl(std::string_view name,
                                 llvm::Type* ret_type,
                                 std::span<llvm::Type* const> arg_types) const;

        template<class... Args>
        void
        get_rt_function(llvm::FunctionCallee* callee,
                        std::string_view name,
                        llvm::Type* ret_type,
                        Args&&... arg_types) const {
            std::array<llvm::Type*, sizeof...(Args)> args{std::forward<Args>(arg_types)...};
            get_rt_function_impl(callee, name, ret_type, args);
        }

        void
        get_rt_function_impl(llvm::FunctionCallee* callee,
                             std::string_view name,
                             llvm::Type* ret_type,
                             std::span<llvm::Type* const> arg_types) const;

        llvm::Type* _c4rt_datum_type;
        llvm::Type* _c4rt_package_type;
        llvm::StructType* _package_struct_type;

        llvm::IRBuilder<llvm::ConstantFolder, llvm::IRBuilderDefaultInserter>* _builder;
        llvm::Module& _module;
    };
}

#endif
