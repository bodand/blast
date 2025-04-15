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
 * src/c4/include/c4/p2/lex/lexer --
 *   
 */
#ifndef C4_LEXER_HXX
#define C4_LEXER_HXX

#include <cstring>
#include <array>
#include <string_view>
#include <concepts>
#include <tuple>

#include <c4/p2/lex/token_source.hxx>
#include <c4/p2/lex/tokens.hxx>
#include <c4/p2/lex/regex_rule.hxx>
#include <c4/diagnostic.hxx>

namespace c4::p2 {
    template<class>
    struct rule_type {
        using type = regex_rule;
    };

    template<class>
    struct rebind_to_lexer_rule_tuple;

    template<template<class...> class L, class... Ts>
    struct rebind_to_lexer_rule_tuple<L<Ts...>> {
        constexpr static auto size = sizeof...(Ts);
        using type = std::tuple<typename rule_type<Ts>::type...>;
        using token_types = std::tuple<Ts...>;
    };

    struct lexer {
        lexer(std::string_view source,
              const char* begin,
              const char* end);

        std::optional<tokens::token_type>
        next();

    private:
        using token_types = rebind_to_lexer_rule_tuple<tokens::token_type>::token_types;

        token_source _token_source;
        position _current_position{&_token_source};
        const char* const _end;
        const char* _data;
        regex_context _regex_context{};
        rebind_to_lexer_rule_tuple<tokens::token_type>::type _rules;
    };
}

#endif
