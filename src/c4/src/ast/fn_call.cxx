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
 * src/c4/src/ast/fn_call --
 *   
 */

#include <c4/evaluation_stack.hxx>
#include <c4/value.hxx>

#include <c4/ast/fn_call.hxx>
#include <c4/ast/op_call.hxx>
#include <c4/ast/let_expression.hxx>

namespace {
    struct call_arity_counter {
        using result_type = unsigned;

        unsigned
        operator()(const c4::ast::call_expr& expr) const {
            return expr.call_arity;
        }

        unsigned
        operator()(const c4::ast::symbol& symbol) const {
            return symbol.arity;
        }
    };

    struct call_evaluator {
        const std::shared_ptr<c4::evaluation_stack>& stk;
        const std::vector<c4::ast::expression>& args;

        call_evaluator(const std::shared_ptr<c4::evaluation_stack>& stk,
                       const std::vector<c4::ast::expression>& args)
            : stk{stk}
            , args{args} { }

        c4::value
        operator()(const c4::ast::call_expr& expr) const {
            const auto callee = expr.expr.evaluate(stk);
            std::vector<const c4::ast::expression*> args;
            std::ranges::transform(this->args,
                                   std::back_inserter(args),
                                   [](auto& arg) { return &arg; });
            return callee.evaluate(stk, args);
        }

        c4::value
        operator()(const c4::ast::symbol& symbol) const {
            auto fn = stk->value_of(c4::symbol::from_ast(symbol));
            std::vector<const c4::ast::expression*> args;
            std::ranges::transform(this->args,
                                   std::back_inserter(args),
                                   [](auto& arg) { return &arg; });
            return (*fn)->evaluate(stk, args);
        }
    };
}

c4::value
c4::ast::fn_call::evaluate(const std::shared_ptr<evaluation_stack>& stk) const {
    return boost::apply_visitor(call_evaluator(stk, args),
                                callee);
}

unsigned
c4::ast::callable::arity() const noexcept {
    return boost::apply_visitor(call_arity_counter(), *this);
}
