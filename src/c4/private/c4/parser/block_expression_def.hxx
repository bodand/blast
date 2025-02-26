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
 * src/c4/private/c4/parser/block_expression_def --
 *   
 */
#ifndef PARSER_BLOCK_EXPRESSION_DEF_HXX
#define PARSER_BLOCK_EXPRESSION_DEF_HXX

#include <boost/spirit/home/x3.hpp>

#include <c4/parser/block_expression.hxx>
#include <c4/parser/expression.hxx>
#include <c4/parser/symbol.hxx>
#include <c4/parser/error_handler_callback.hxx>
#include <c4/parser/position_annotator.hxx>

namespace c4::parser {
    namespace x3 = boost::spirit::x3;

    struct block_expression_parser;
    struct block_parameters_parser;

    constexpr block_expression_parser_type block_expression_parser = "block expression";
    constexpr x3::rule<block_parameters_parser, ast::block_parameters> block_parameters_parser = "block parameters";

    const auto push_scope = [](auto&& ctx) {
        auto& current_scope = x3::get<symbol_scope_tag>(ctx).get();
        x3::get<symbol_scope_tag>(ctx) = *current_scope.new_scope();
    };

    const auto pop_scope = [](auto&& ctx) {
        auto& current_scope = x3::get<symbol_scope_tag>(ctx).get();
        x3::get<symbol_scope_tag>(ctx) = current_scope.parent();
    };

    constexpr auto block_parameters_parser_def =
            "|" >> *symbol() > "|";

    constexpr auto block_expression_parser_def =
            x3::eps[push_scope]
            >> ("{" >> -block_parameters_parser >> *expression() > "}"
                | "\\" >> -block_expression_parser > expression())
            >> x3::eps[pop_scope];

    struct block_expression_parser : position_annotator,
                                     error_handler_callback { };

    struct block_parameters_parser : position_annotator,
                                     error_handler_callback { };

    BOOST_SPIRIT_DEFINE(block_parameters_parser, block_expression_parser)

    constexpr block_expression_parser_type
    block_expression() { return block_expression_parser; }
}


#endif
