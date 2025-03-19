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
 * src/c4/include/c4/p2/lex/token_traits --
 *   A set of utilities for metaprogramming with token types.
 */
#ifndef TOKEN_TRAITS_HXX
#define TOKEN_TRAITS_HXX

#include <concepts>
#include <type_traits>
#include <cstdint>
#include <string_view>

namespace c4::p2 {
    template<class T>
    concept regex_token = requires(T token)
    {
        { T::regex } -> std::same_as<std::string_view>;
    };

    template<auto>
    using void_val_t = void;

    template<class, class = void>
    struct token_can_match_newline : std::false_type { };

    template<class Token>
    struct token_can_match_newline<Token,
                             void_val_t<Token::can_match_newline>>
            : std::bool_constant<Token::can_match_newline> { };

    template<class, class = void>
    struct token_capture_groups
            : std::integral_constant<std::uint32_t, 0 + 1> { };

    template<class Token>
    struct token_capture_groups<Token,
                          void_val_t<Token::group_count>>
            : std::integral_constant<std::uint32_t, Token::group_count + 1> { };

}

#endif
