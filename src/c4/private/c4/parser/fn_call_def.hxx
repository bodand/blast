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
 * src/c4/private/c4/parser/fn_call_def --
 *   
 */
#ifndef PARSER_FN_CALL_DEF_HXX
#define PARSER_FN_CALL_DEF_HXX

#include <boost/spirit/home/x3.hpp>

#include <c4/ast/fn_call_fusion.hxx>
#include <c4/parser/fn_call.hxx>
#include <c4/parser/symbol.hxx>
#include <c4/parser/expression.hxx>
#include <c4/parser/error_handler_callback.hxx>
#include <c4/parser/position_annotator.hxx>
#include <c4/parser/valid_symbol_parser.hxx>

namespace c4::parser {
    namespace x3 = boost::spirit::x3;

    struct dynamic_expression_parser_t : x3::parser<dynamic_expression_parser_t> {
        using attribute_type = ast::fn_call;

        template<class It, class Ctx, class RCtx, class Attr>
        bool
        parse(It& begin,
              It end,
              Ctx& ctx,
              RCtx&& rctx,
              Attr& attr) const;
    };

    constexpr static dynamic_expression_parser_t dynamic_expression_parser;

    struct fn_call_parser;
    struct call_expr_parser;
    struct callable_parser;

    constexpr fn_call_parser_type fn_call_parser = "function call";
    constexpr x3::rule<call_expr_parser, ast::call_expr> call_expr_parser = "call expression";
    constexpr x3::rule<callable_parser, ast::callable> callable_parser = "callable";

    const auto call_expr_parser_def =
            "&" > expression() > x3::lexeme["/" > x3::uint_];

    constexpr auto callable_parser_def =
            call_expr_parser
            | valid_symbol_parser;

    constexpr auto fn_call_parser_def = dynamic_expression_parser;

    struct fn_call_parser : position_annotator
                            , error_handler_callback { };

    struct call_expr_parser : position_annotator
                              , error_handler_callback { };

    struct callable_parser : position_annotator
                             , error_handler_callback { };

    BOOST_SPIRIT_DEFINE(fn_call_parser, call_expr_parser, callable_parser)

    constexpr fn_call_parser_type
    fn_call() { return fn_call_parser; }

    template<class It, class Ctx, class RCtx, class Attr>
    bool
    dynamic_expression_parser_t::parse(It& begin, It end,
                                       Ctx& ctx,
                                       RCtx&& rctx,
                                       Attr& attr) const {
        It memory = begin;

        if (!callable_parser.parse(begin, end,
                                   std::forward<Ctx>(ctx),
                                   std::forward<RCtx>(rctx),
                                   attr.callee)) {
            begin = memory;
            return false;
        }

        const auto arity = attr.callee.arity();
        if (const auto args_parser = x3::as_parser(
                x3::repeat(arity)[x3::expect[expression()]]);
            !args_parser.parse(begin, end,
                               std::forward<Ctx>(ctx),
                               std::forward<RCtx>(rctx),
                               attr.args)) {
            begin = memory;
            return false;
        }

        return true;
    }
}

#endif
