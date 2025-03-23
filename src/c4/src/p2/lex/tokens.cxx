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
 * src/c4/src/p2/ley --
 *   
 */

#include <charconv>

#include <c4/p2/lex/tokens.hxx>
#include <c4/p2/lex/token_source.hxx>

#include <uni_algo/ranges.h>
#include <uni_algo/ranges_conv.h>

#include <libassert/assert.hpp>

std::string_view
c4::p2::tokens::token_base::source_name() const noexcept {
    return _source->file_string();
}

c4::p2::tokens::token_base::token_base(token_source* source,
                                       const position& position,
                                       const std::string_view range)
    : _position(position)
    , _begin(range.data())
    , _end(range.data() + range.size())
    , _source(source) {
    DEBUG_ASSERT(source != nullptr, "source cannot be specified as null");
    DEBUG_ASSERT(_begin <= _end, "token must start before it ends");
    DEBUG_ASSERT(_begin != _end, "token must not be empty");
}

namespace {
    template<class T>
    T
    parse_number(const std::string_view str) {
        T ret{};
        auto [pos, errc] = std::from_chars(str.data(), str.data() + str.size(), ret);
        ASSERT(errc == std::errc{},
               "parse_uint must be called on guaranteed input: the result of the lexer: this was not the case",
               str);
        return ret;
    }
}

c4::p2::tokens::symbol::symbol(token_source* source,
                               const position& position,
                               const std::string_view range,
                               const std::string_view name_range,
                               const std::string_view arity_range)
    : token_base{source, position, range}
    , _name_begin{name_range.data()}
    , _name_end{name_range.data() + name_range.size()}
    , _arity{parse_number<unsigned>(arity_range)} {
    DEBUG_ASSERT(_name_begin <= _name_end, "token name must start before it ends");
    DEBUG_ASSERT(_begin <= _name_begin, "name must begin within the token");
    DEBUG_ASSERT(_name_begin < _end, "name must begin within the token");
    DEBUG_ASSERT(_name_begin <= _name_end, "name must end within the token");
    DEBUG_ASSERT(_name_end <= _end, "name must end within the token");
    DEBUG_ASSERT(_name_begin != _name_end, "name must not be empty");
}

#define STR_I(x) #x
#define STR(x) STR_I(x)

#define C4P2_DEFAULT_SYMBOL_TOKEN(name, ch) \
c4::p2::tokens::name::name(token_source* source, \
                           const position& position, \
                           const std::string_view range) \
    : token_base{source, position, range} { \
    DEBUG_ASSERT(_end - _begin == sizeof(ch) - 1, STR(name) " token must have a given number of characters"); \
    DEBUG_ASSERT(std::string_view(_begin, sizeof(ch) - 1) == (ch), STR(name) " token must represent " STR(ch)); \
}

C4P2_DEFAULT_SYMBOL_TOKEN(lbrace, "{")
C4P2_DEFAULT_SYMBOL_TOKEN(rbrace, "}")
C4P2_DEFAULT_SYMBOL_TOKEN(lparen, "(")
C4P2_DEFAULT_SYMBOL_TOKEN(rparen, ")")
C4P2_DEFAULT_SYMBOL_TOKEN(pipe, "|")
C4P2_DEFAULT_SYMBOL_TOKEN(ampersand, "&(")
C4P2_DEFAULT_SYMBOL_TOKEN(semicolon, ";")
C4P2_DEFAULT_SYMBOL_TOKEN(backslash, "\\")
C4P2_DEFAULT_SYMBOL_TOKEN(let, "let")

c4::p2::tokens::arity_marker::arity_marker(token_source* source,
                                           const position& pos,
                                           const std::string_view range,
                                           const std::string_view arity_range)
    : token_base{source, pos, range}
    , _arity{parse_number<unsigned>(arity_range)} { }

c4::p2::tokens::integer_literal::integer_literal(token_source* source,
                                                 const position& position,
                                                 const std::string_view range)
    : token_base{source, position, range}
    , _int_value{parse_number<std::int64_t>(range)} { }


c4::p2::tokens::float_literal::float_literal(token_source* source,
                                             const position& position,
                                             const std::string_view range)
    : token_base{source, position, range}
    , _float_value{parse_number<double>(range)} { }

