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
 * src/c4/include/c4/ast2/string_literal --
 *   
 */
#ifndef C4_AST2_STRING_LITERAL_HXX
#define C4_AST2_STRING_LITERAL_HXX

#include <string_view>

#include <c4/ast2/tags/visitable.hxx>
#include <c4/ast2/tags/source_positioned.hxx>
#include <c4/ast2/tags/attributable.hxx>
#include <c4/ast2/tags/evaluation_constness.hxx>

namespace c4::ast2 {
    struct string_literal final : tags::visitable
                                  , tags::source_positioned
                                  , tags::evaluation_constness
                                  , tags::attributable {
        constexpr static auto short_string_limit = 6;

        string_literal(const c4::position& position,
                       const std::string_view file_source,
                       const std::size_t length,
                       const std::string_view value)
            : source_positioned{position, file_source, length}
            , _value{value} { }

        [[nodiscard]] std::string_view
        value() const { return _value; }

        [[nodiscard]] bool
        is_constant_evaluable() const noexcept { return _value.size() < short_string_limit; }

    private:
        std::string_view _value;
    };
}

#endif
