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
 * src/c4/private/c4/parser/valid_symbol_parser --
 *   
 */
#ifndef PARSER_VALID_SYMBOL_PARSER_HXX
#define PARSER_VALID_SYMBOL_PARSER_HXX

#include <boost/spirit/home/x3.hpp>

#include <c4/ast/symbol.hxx>

namespace c4::parser {
    namespace x3 = boost::spirit::x3;

    struct valid_symbol_parser_t : x3::parser<valid_symbol_parser_t> {
        using attribute_type = ast::symbol;

        template<class It, class Ctx, class RCtx, class Attr>
        bool
        parse(It& begin,
              It end,
              Ctx& ctx,
              RCtx&& rctx,
              Attr& attr) const {
            auto& scope = x3::get<symbol_scope_tag>(ctx).get();
            x3::symbols<attribute_type> symbols;
            scope.load_parser(symbols);

            It memory = begin;

            x3::skip_over(begin, end, ctx);
            if (symbols.parse(begin, end,
                              std::forward<Ctx>(ctx),
                              std::forward<RCtx>(rctx),
                              attr)) {
                return true;
            }

            begin = memory;
            return false;
        }
    };

    constexpr static valid_symbol_parser_t valid_symbol_parser;
}

#endif
