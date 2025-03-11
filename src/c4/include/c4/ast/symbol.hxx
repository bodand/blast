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
 * Originally created: 2025-02-26.
 *
 * src/c4/ast/symbol --
 *   
 */
#ifndef AST_SYMBOL_HXX
#define AST_SYMBOL_HXX

#include <ostream>
#include <string>

#include <boost/spirit/home/x3/support/ast/position_tagged.hpp>

namespace c4::ast {
    namespace x3 = boost::spirit::x3;

    struct symbol : x3::position_tagged {
        std::string name;
        unsigned arity;

        symbol(const std::string& name = {}, unsigned arity = {})
            : name{name}
            , arity{arity} { }

        friend bool
        operator==(const symbol& lhs, const symbol& rhs) {
            return lhs.name == rhs.name && lhs.arity == rhs.arity;
        }

        friend bool
        operator!=(const symbol& lhs, const symbol& rhs) { return !(lhs == rhs); }

        friend std::ostream&
        operator<<(std::ostream& os, const symbol& obj) {
            return os << obj.name << "/" << obj.arity;
        }
    };
}

template<>
struct std::hash<c4::ast::symbol> {
    std::size_t
    operator()(const c4::ast::symbol& obj) const noexcept {
        std::size_t seed = 0x484DB776;
        seed ^= (seed << 6) + (seed >> 2) + 0x53385480 + hash<std::string>()(obj.name);
        seed ^= (seed << 6) + (seed >> 2) + 0x1D082437 + static_cast<std::size_t>(obj.arity);
        return seed;
    }
};

#endif
