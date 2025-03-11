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
 * src/c4/include/c4/symbol --
 *   This is a runtime representation of a symbol.
 *   Differs from ast/symbol by virtue of not being bound to the source code.
 *   The symbol in ast/symbol represents symbols arising from the parsing of the
 *   source code, while this symbol refers to all symbols that are generated
 *   during runtime.
 */
#ifndef SYMBOL_HXX
#define SYMBOL_HXX

#include <string>
#include <utility>

namespace c4 {
    namespace ast {
        struct symbol;
    }

    struct symbol {
        std::string name;
        unsigned arity;

        symbol(std::string name, const unsigned arity)
            : name{std::move(name)}
            , arity{arity} { }

        [[nodiscard]] static symbol
        from_ast(const ast::symbol& sym);

        [[nodiscard]] static symbol
        argument(size_t i);

    private:
        friend bool
        operator==(const symbol& lhs, const symbol& rhs) = default;

        friend bool
        operator!=(const symbol& lhs, const symbol& rhs) = default;
    };
}

template<>
struct std::hash<c4::symbol> {
    std::size_t
    operator()(const c4::symbol& obj) const noexcept {
        std::size_t seed = 0x1BF1A69F;
        seed ^= (seed << 6) + (seed >> 2) + 0x36195B63 + std::hash<std::string>()(obj.name);
        seed ^= (seed << 6) + (seed >> 2) + 0x3ACC40D6 + static_cast<std::size_t>(obj.arity);
        return seed;
    }
};

#endif
