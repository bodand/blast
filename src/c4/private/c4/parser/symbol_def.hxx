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
 * Originally created: 2025-02-02.
 *
 * src/c4/_private/parser/symbol_def --
 *   
 */
#ifndef SYMBOL_DEF_HXX
#define SYMBOL_DEF_HXX

#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/directive.hpp>

#include <c4/ast/symbol.hxx>
#include <c4/ast/symbol_fusion.hxx>
#include <c4/parser/error_handler_callback.hxx>
#include <c4/parser/position_annotator.hxx>
#include <c4/parser/symbol.hxx>

#define C4_PARSER_SYMBOL_HEAD_CHARS "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_"
#define C4_PARSER_SYMBOL_CHARS C4_PARSER_SYMBOL_HEAD_CHARS "0123456789'"

namespace c4::parser {
    namespace x3 = boost::spirit::x3;

    struct symbol_parser;
    struct bare_symbol_parser;
    struct symbol_name;

    constexpr symbol_parser_type symbol_parser = "symbol";
    constexpr bare_symbol_parser_type bare_symbol_parser = "bare symbol";
    constexpr x3::rule<symbol_name, std::string> symbol_name = "symbol name";

    struct symbol_parser : position_annotator
                           , error_handler_callback {};

    struct bare_symbol_parser : position_annotator
                                , error_handler_callback {};

    struct symbol_name : position_annotator
                         , error_handler_callback {};

    // clang-format off
    const auto symbol_name_def =
            x3::lexeme[x3::char_(C4_PARSER_SYMBOL_HEAD_CHARS)
                       >> *x3::char_(C4_PARSER_SYMBOL_CHARS)];

    constexpr auto symbol_parser_def = x3::lexeme[symbol_name > '/' > x3::uint_];

    constexpr auto bare_symbol_parser_def = symbol_name >> x3::attr(0U);
    // clang-format on

    BOOST_SPIRIT_DEFINE(symbol_parser, bare_symbol_parser, symbol_name)

    inline symbol_parser_type
    symbol() { return symbol_parser; }

    inline bare_symbol_parser_type
    bare_symbol() { return bare_symbol_parser; }
}

#endif
