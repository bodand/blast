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
 * src/c4/include/c4/ast/op_call --
 *   
 */
#ifndef AST_OP_CALL_HXX
#define AST_OP_CALL_HXX

#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <boost/spirit/home/x3/support/ast/position_tagged.hpp>

#include <c4/ast/block_expression.hxx>
#include <c4/ast/expression.hxx>
#include <c4/ast/fn_call.hxx>
#include <c4/ast/fundamental_scalar.hxx>
#include <c4/ast/symbol.hxx>
#include <c4/value.hxx>

#include <vector>

namespace c4 {
    struct evaluation_stack;
}

namespace c4::ast {
    namespace x3 = boost::spirit::x3;

    template<unsigned Precedence>
    struct precedence_op_expr : x3::position_tagged {
        using next_precedence = precedence_op_expr<Precedence - 1>;

        struct op_chain {
            std::string op;
            next_precedence precedence;
        };

        next_precedence next;
        std::vector<op_chain> ops;
    };

    template<>
    struct precedence_op_expr<0> : x3::position_tagged,
                                   x3::variant<
                                       expression, // from '(' <expr> ')'
                                       fundamental_scalar,
                                       symbol,
                                       block_expression,
                                       // TODO prefix expr
                                       fn_call
                                   > {
        using base_type::base_type;
        using base_type::operator=;

        [[nodiscard]] value
        evaluate(const std::shared_ptr<evaluation_stack>& stk) const;
    };

    struct op_call : x3::position_tagged {
        precedence_op_expr<0> expression;

        [[nodiscard]] value
        evaluate(const std::shared_ptr<evaluation_stack>& stk) const;
    };
}

#endif
