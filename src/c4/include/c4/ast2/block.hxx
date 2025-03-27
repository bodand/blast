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
 * src/c4/include/c4/ast2/block --
 *   
 */
#ifndef C4_AST2_BLOCK_HXX
#define C4_AST2_BLOCK_HXX

#include <c4/ast2/tags/clonable.hxx>
#include <c4/ast2/tags/source_positioned.hxx>

#include <c4/ast2/expression_deleter.hxx>
#include <c4/ast2/symbol.hxx>

#include <span>
#include <optional>
#include <vector>
#include <c4/ast2/tags/visitable.hxx>

namespace c4::ast2 {
    struct block_args : tags::clonable
                        , tags::visitable
                        , tags::source_positioned {
        block_args(const c4::position& position,
                   std::string_view file_source,
                   std::size_t length,
                   std::span<symbol> args);

        [[nodiscard]] std::span<const symbol>
        args() const { return {_args}; }

    private:
        std::vector<symbol> _args{};
    };

    struct block final : tags::clonable
                         , tags::visitable
                         , tags::source_positioned {
        block(const c4::position& position,
              std::string_view file_source,
              std::size_t length,
              block_args&& args,
              std::span<expression> expressions);

        block(const c4::position& position,
              std::string_view file_source,
              std::size_t length,
              std::span<expression> expressions);

        [[nodiscard]] std::optional<block_args>
        args() const { return _args; }

        [[nodiscard]] std::span<const expression>
        expressions() const;

    private:
        std::optional<block_args> _args{};
        std::vector<expression> _expressions;
    };
}

#endif
