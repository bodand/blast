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
 * src/c4/include/c4/p2/lex/string_rule --
 *   todo
 */
#ifndef C4_P2_STRING_RULE_HXX
#define C4_P2_STRING_RULE_HXX

#include <optional>
#include <string_view>
#include <c4/diagnostic.hxx>

#include "token_source.hxx"

namespace c4 {
    struct position;
}

namespace c4::p2 {
    struct string_rule {
        explicit
        string_rule(token_source& token_source,
                    const std::string_view str,
                    auto&&... /*ignore*/)
            : _str{str}
            , _token_source{&token_source} {
            _match_newline = _str.find('\n') != std::string_view::npos;
        }

        template<class T>
        std::optional<T>
        match(const char*& data, const char* end, position& pos) const {
            if (std::cmp_less(end - data, _str.size())) return std::nullopt;

            const auto matchee = std::string_view{data, _str.size()};
            if (matchee != _str) return std::nullopt;

            auto matched_at = pos.snapshot();
            update_matched_position(data, end, pos, matched_at);

            return _token_source->build<T>(matched_at, matchee);
        }

    private:
        void
        update_matched_position(const char*& data,
                                const char* end,
                                position& pos,
                                position& matched_at) const;

        void
        offset_positions_newline(const char*& begin,
                                 const char* end,
                                 position& pos) const;

        void
        offset_positions_no_newline(const char*& begin,
                                    const char* end,
                                    position& pos) const;

        std::string_view _str;
        bool _match_newline;
        token_source* _token_source;
    };
}

#endif
