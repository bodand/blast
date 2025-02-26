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
 * src/c4/private/c4/parser/op_call_def --
 *   
 */
#ifndef PARSER_OP_CALL_DEF_HXX
#define PARSER_OP_CALL_DEF_HXX

#include <boost/spirit/home/x3.hpp>
#include <c4/parser/block_expression.hxx>

#include <c4/parser/symbol.hxx>
#include <c4/parser/expression.hxx>
#include <c4/parser/error_handler_callback.hxx>
#include <c4/parser/fn_call.hxx>
#include <c4/parser/fundamental_scalar.hxx>
#include <c4/parser/op_call.hxx>
#include <c4/parser/position_annotator.hxx>
#include <c4/parser/valid_symbol_parser.hxx>

#include <format>

namespace c4::parser {
    namespace x3 = boost::spirit::x3;

    struct op_call_parser;
    template<unsigned Precedence>
    struct precedence_op_expr_parser;

    constexpr op_call_parser_type op_call_parser = "operator call";

    constexpr x3::rule<precedence_op_expr_parser<0>, ast::precedence_op_expr<0>>
    precedence_op_expr0 = "operand expression";

    constexpr auto precedence_op_expr0_def =
            "(" >> expression() > ")"
            | fundamental_scalar()
            | symbol()
            | block_expression()
            | fn_call();

    constexpr auto op_call_parser_def = precedence_op_expr0_def;

    struct op_call_parser : position_annotator,
                            error_handler_callback { };

    template<>
    struct precedence_op_expr_parser<0> : position_annotator,
                                          error_handler_callback { };

    BOOST_SPIRIT_DEFINE(op_call_parser, precedence_op_expr0)

    constexpr op_call_parser_type
    op_call() { return op_call_parser; }
}

#endif
