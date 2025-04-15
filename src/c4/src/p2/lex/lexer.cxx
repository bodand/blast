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
 * src/c4/src/p2/lex/lexer --
 *   
 */

#include <c4/p2/lex/lexer.hxx>
#include <c4/p2/lex/token_traits.hxx>

namespace {
    template<class>
    struct ruleset_builder;

    template<template<class...> class L, class... Ts>
    struct ruleset_builder<L<Ts...>> {
        constexpr static auto size = sizeof...(Ts);
        using type = std::tuple<typename c4::p2::rule_type<Ts>::type...>;

        static type
        build(c4::p2::token_source& token_source,
              c4::p2::regex_context& regex_context) {
            return {
                c4::p2::regex_rule(
                    token_source,
                    regex_context,
                    Ts::regex,
                    c4::p2::token_capture_groups<Ts>::value
                )...
            };
        }
    };

    constexpr auto linebreak_markers = std::string_view("\n\0", 2U);
}

c4::p2::lexer::lexer(const std::string_view source,
                     const char* begin, const char* end)
    : _token_source{source}
    , _end{end}
    , _data{begin}
    , _rules{ruleset_builder<tokens::token_type>::build(_token_source, _regex_context)} {
    const auto buffer = std::string_view(begin, end);
    const auto it = std::ranges::find_first_of(buffer, linebreak_markers);
    _current_position.line = buffer.substr(0, distance(std::next(std::begin(buffer)), it));
}

namespace {
    struct matcher {
        matcher(const char*& begin, const char* end, c4::position& pos)
            : begin{begin}
            , end{end}
            , pos{pos} { }

        template<class T>
        bool
        do_match(const auto& rule) {
            ret = rule.template match<T>(begin, end, pos);
            return ret.has_value();
        }

        const char* & begin;
        const char* end;
        c4::position& pos;
        std::optional<c4::p2::tokens::token_type> ret;
    };
}

std::optional<c4::p2::tokens::token_type>
c4::p2::lexer::next() {
    if (_data == _end) return {};

    auto m = matcher(_data, _end, _current_position);
    std::ignore =
            [this, &m]<std::size_t... Is>(std::index_sequence<Is...>) {
                return (m.do_match<std::tuple_element_t<Is, token_types>>(std::get<Is>(_rules)) || ...);
            }(std::make_index_sequence<std::tuple_size_v<decltype(_rules)>>{});
    return m.ret;
}
