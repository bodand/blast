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
 * src/c4c/include/c4c/global_constant_emitter --
 *   
 */
#ifndef C4C_GLOBAL_CONSTANT_EMITTER_HXX
#define C4C_GLOBAL_CONSTANT_EMITTER_HXX

#include <string_view>

#include <c4/ast2/visitor/visitor.hxx>

#include <c4rt/datum.h>

namespace llvm {
    class Twine;
    class Module;
    class LLVMContext;
    class Value;
    class ConstantFolder;
    class IRBuilderDefaultInserter;
    template<typename FolderTy, typename InserterTy>
    class IRBuilder;
}

namespace c4::ast2 {
    struct string_literal;
    struct integer_literal;
    struct float_literal;
}

namespace c4c {
    struct global_constant_emitter final : c4::ast2::visitor<
                c4::ast2::float_literal,
                c4::ast2::integer_literal,
                c4::ast2::string_literal
            > {
        global_constant_emitter(llvm::LLVMContext& context,
                                llvm::Module& module,
                                llvm::IRBuilder<llvm::ConstantFolder, llvm::IRBuilderDefaultInserter>& builder);

        llvm::Value*
        get_loaded_global(const llvm::Twine& name);

        void
        do_visit(const c4::ast2::float_literal& obj) override;

        void
        do_visit(const c4::ast2::integer_literal& obj) override;

        void
        do_visit(const c4::ast2::string_literal& obj) override;

        llvm::Value* value{};
        llvm::LLVMContext& context;
        llvm::Module& module;
        llvm::IRBuilder<llvm::ConstantFolder, llvm::IRBuilderDefaultInserter>& builder;

    private:
        std::string_view _value_type_suffix;

        void
        create_global(c4_datum_t datum);
    };
}

#endif
