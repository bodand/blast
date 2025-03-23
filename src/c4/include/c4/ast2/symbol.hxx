/* demo project
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
 * src/c4/include/c4/ast2/symbol --
 *   Either a bare-symbol (sym) or an elaborated symbol (sym/1) are stored here.
 *   The latter can stand by itself in a lot of spaces, but the former is mostly
 *   present as elements of let-expressions or fn-calls.
 */
#ifndef C4_AST2_SYMBOL_HXX
#define C4_AST2_SYMBOL_HXX

#include <string>
#include <cmath>

#include <c4/ast2/tags/clonable.hxx>
#include <c4/ast2/tags/source_positioned.hxx>

namespace c4::ast2 {
    struct symbol final : tags::clonable
                          , tags::source_positioned {
        symbol(const c4::position& position,
               std::string_view file_source,
               std::size_t length,
               std::string_view name,
               unsigned arity);

        [[nodiscard]] std::string_view
        name() const noexcept { return _name; }

        [[nodiscard]] unsigned
        arity() const noexcept { return _arity; }

    private:
        std::string_view _name;
        unsigned _arity;
    };

    struct op_symbol final : tags::clonable
                             , tags::source_positioned {
        op_symbol(const c4::position& position,
                  std::string_view file_source,
                  std::size_t length,
                  std::string_view name,
                  unsigned arity);

        [[nodiscard]] std::string_view
        name() const noexcept { return _name; }

        [[nodiscard]] unsigned
        arity() const noexcept { return _arity; }

    private:
        std::string_view _name;
        unsigned _arity;
    };
}

#endif
