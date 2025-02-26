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
 * Originally created: 2025-02-17.
 *
 * src/parser/parser --
 *   
 */
#ifndef DEMO_PARSER_DEF_HXX
#define DEMO_PARSER_DEF_HXX

#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/utility/error_reporting.hpp>
#include <boost/spirit/home/x3/directive.hpp>

#include <iostream>

#include "ast.hxx"
#include "ast_adapted.hxx"
#include "parser.hxx"

namespace demo {
  namespace parser {
    namespace x3 = boost::spirit::x3;
    namespace ascii = boost::spirit::x3::ascii;

    using x3::int_;
    using x3::lit;
    using x3::eps;
    using x3::any_parser;
    using x3::lexeme;
    using x3::expect;
    using ascii::char_;
    using ascii::alpha;
    using ascii::alnum;

    struct symbol_value;
    struct operator_value;
    struct symbol;
    struct expression;
    struct let_expression;
    struct parameter_list;
    struct block_expression;
    struct symbol_expression;

    const x3::rule<script_parser, ast::script> script = "script";

    auto const quoted_string =
            lexeme['"' >> *(char_ - '"') >> '"'];

    const x3::rule<symbol_value, std::string> symbol_value = "symbol_value";

    auto const symbol_value_def =
            lexeme[(alpha | "_" | "'") >> *(alnum | "_" | "'")];

    const x3::rule<operator_value, std::string> operator_value = "operator";

    auto const operator_value_def =
            lexeme[+char_("+-*/<>@&|!%=`~")];

    const x3::rule<expression, ast::expression> expression = "expression";

    struct symbol_call_parser_t : x3::parser<symbol_call_parser_t> {
        using attribute_type = ast::let_expression;

        template<class It, class Ctx, class RCtx, class Attr>
        bool
        parse(It& begin,
              It end,
              Ctx& ctx,
              RCtx&& rctx,
              Attr& attr) const {
            auto& symbols = x3::get<ast::symbol_table_tag>(ctx).get();
            It memory = begin;

            x3::skip_over(begin, end, ctx);
            if (symbols.parser.parse(begin, end,
                                     std::forward<Ctx>(ctx),
                                     std::forward<RCtx>(rctx),
                                     attr.symbol)) {
                auto arity = symbols.arity_of(attr.symbol);
                if (x3::as_parser(x3::repeat(arity)[expect[expression]])
                        .parse(begin, end,
                               std::forward<Ctx>(ctx),
                               std::forward<RCtx>(rctx),
                               attr.arguments)) {
                    return true;
                }
            }

            begin = memory;
            return false;
        }
    };

    constexpr const static symbol_call_parser_t symbol_call_parser{};

    const x3::rule<symbol, ast::symbol> symbol_decl = "symbol_decl";
    const x3::rule<let_expression, ast::let_expression> let_expression = "let_expression";
    const x3::rule<parameter_list, ast::block_parameters> parameter_list = "parameter_list";
    const x3::rule<block_expression, ast::block_expression> block_expression = "block_expression";
    const x3::rule<symbol_expression, ast::symbol_expression> symbol_expression = "symbol_expression";

    // clang-format off
    const auto symbol_decl_def =
            lexeme[symbol_value >> -("/" > int_)];

    const auto expression_def =
            quoted_string
            | let_expression
            | block_expression
            | symbol_expression
            ;

    const auto parameter_list_def =
            '|' >> *symbol_decl >> expect['|'];

    const auto block_expression_def =
            '{'
            >> -parameter_list
            >> *expression
            >>
            expect['}'];

    const auto symbol_expression_def =
            symbol_call_parser;

    const auto let_expression_def =
            "let"
            >> expect[symbol_value]
            >> expect[expression];

    auto const script_def = *expression;
    // clang-format on

    struct position_annotator {
        template<class T, class It, class Ctx>
        void
        on_success(const It& begin,
                   const It& end,
                   T& ast,
                   const Ctx& ctx) {
            auto& pos = x3::get<ast::position_cache_tag>(ctx).get();
            pos.annotate(ast, begin, end);
        }
    };

    struct error_handler_callback {
        template<class It, class Exception, class Ctx>
        x3::error_handler_result
        on_error(It& first,
                 const It& last,
                 const Exception& x,
                 const Ctx& ctx) {
            auto& error_handler = x3::get<x3::error_handler_tag>(ctx).get();
            std::string message = "error: expecting " + x.which() + " here:";
            error_handler(x.where(), message);
            return x3::error_handler_result::fail;
        }
    };

    struct arity_counter {
        using result_type = std::size_t;

        result_type
        operator()(const std::string&) const { return 0; }

        result_type
        operator()(const demo::ast::symbol_expression&) const { return 0; }

        result_type
        operator()(const demo::ast::let_expression&) const { return 0; }

        result_type
        operator()(const demo::ast::block_expression& block) const { return block.arity(); }
    };

    struct script_parser : position_annotator,
                           error_handler_callback {
    };
    struct symbol_value : position_annotator,
                          error_handler_callback {
    };
    struct symbol : position_annotator,
                    error_handler_callback {
    };
    struct expression : position_annotator,
                        error_handler_callback {
    };
    struct parameter_list : position_annotator,
                            error_handler_callback {
    };
    struct block_expression : position_annotator,
                              error_handler_callback {
    };
    struct symbol_expression : position_annotator,
                               error_handler_callback {
    };

    struct let_expression : position_annotator,
                            error_handler_callback {
        template<class T, class It, class Ctx>
        void
        on_success(const It& begin,
                   const It& end,
                   T& ast,
                   const Ctx& ctx) {
            position_annotator::on_success(begin, end, ast, ctx);
            auto& symbols = x3::get<ast::symbol_table_tag>(ctx).get();
            symbols.add_symbol(ast.symbol, boost::apply_visitor(arity_counter{}, ast.expr));
        }
    };

    BOOST_SPIRIT_DEFINE(script,
                        symbol_value,
                        symbol_decl,
                        let_expression,
                        parameter_list,
                        block_expression,
                        symbol_expression,
                        expression);
  }

  parser::script_type
  script() {
      return parser::script;
  }
}

#endif
