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
 * src/c4/src/ast/expression --
 *   
 */

#include <c4/evaluation_stack.hxx>
#include <c4/ast/expression.hxx>
#include <c4/ast/op_call.hxx>
#include <c4/ast/let_expression.hxx>
#include <c4/value.hxx>

namespace {
    struct expression_evaluator final {
        const std::shared_ptr<c4::evaluation_stack>& stk;

        c4::value
        operator()(std::monostate) const {
            // reaching this is sus?
            return c4::value::nil();
        }

        c4::value
        operator()(const c4::ast::op_call& op) const {
            return op.evaluate(stk);
        }

        c4::value
        operator()(const c4::ast::let_expression& le) const {
            stk->set(c4::symbol::from_ast(le.symbol), &le.expression);
            return c4::symbol::from_ast(le.symbol);
        }
    };
}

c4::value
c4::ast::expression::evaluate(const std::shared_ptr<evaluation_stack>& stk) const {
    return boost::apply_visitor(expression_evaluator(stk), *this);
}
