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
 * src/c4/src/ast2/symbol --
 *   
 */

#include <iostream>
#include <c4/ast2/symbol.hxx>

#include <fmt/base.h>
#include <libassert/assert.hpp>

using namespace std::literals;

c4::ast2::symbol::symbol(const c4::position& position,
                         const std::string_view file_source,
                         const std::size_t length,
                         const std::string_view name,
                         const unsigned arity)
    : source_positioned{
        position,
        file_source,
        length
    }
    , _name{name}
    , _arity{arity} {
    DEBUG_ASSERT(!_name.empty(), "symbol name must not be empty");
}

namespace {
    std::string
    mangle_symbol(std::string_view name, const unsigned arity) {
        // clang-format off
        constexpr static auto operator_chars =            "-+*/%&|~!?,.:^@#`<>="sv;
        constexpr static auto operator_char_replacement = "mptsPaoTeqcdChAHblgE"sv;
        // clang-format on
        static_assert(operator_chars.size() == operator_char_replacement.size(),
                      "replacement set must equal operator set");

        if (const auto idx = name.find_first_of(operator_chars);
            idx == std::string_view::npos)
            return fmt::format("{}{}{}", name.size(), name, arity);

        auto normalized = std::string(name);
        std::ranges::transform(normalized, normalized.begin(),
                               [](const char c) {
                                   const auto c_idx = operator_chars.find(c);
                                   if (c_idx == std::string_view::npos) return c;
                                   return operator_char_replacement[c_idx];
                               });
        return fmt::format("op{}{}{}", normalized.size(), normalized, arity);
    }
}

c4::ast2::symbol
c4::ast2::symbol::with_arity(const unsigned arity) const {
    auto cpy = *this;
    cpy._arity = arity;
    return cpy;
}

std::string
c4::ast2::symbol::mangle() const { return mangle_symbol(_name, _arity); }

std::string
c4::ast2::undef_symbol::mangle() const { return mangle_symbol(_name, _arity); }

void
c4::ast2::symbol::references(referable* ref) noexcept {
    if (ref == nullptr) {
        _references = nullptr;
        return;
    }

    DEBUG_ASSERT(ref->name() == _name,
                 "referenced entity must have the same name");
    DEBUG_ASSERT((!_references || _references == ref),
                 "referenced value should not be overwritten");
    _references = ref;
}
