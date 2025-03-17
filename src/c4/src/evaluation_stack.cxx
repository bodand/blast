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
 * src/c4/src/evaluation_stack --
 *   
 */

#include <c4/evaluation_stack.hxx>
#include <c4/value.hxx>
#include <c4/ast/expression.hxx>

void
c4::evaluation_stack::merge(evaluation_stack& other) {
    for (auto& [symbol, value]: other._symbol_values) {
        // ignore failure
        _symbol_values.try_emplace(symbol, static_cast<referenced_value>(&value));
    }
}

void
c4::evaluation_stack::set(const symbol& sym, value&& val) {
    if (const auto it = _symbol_values.find(sym);
        it == _symbol_values.end()) {
        _symbol_values.emplace(sym, std::move(val));
    }
    // else {
    //     _symbol_values[sym] = std::move(val);
    // }
}

void
c4::evaluation_stack::set(const symbol& sym, const ast::expression* expr) {
    auto& eval_stack = _parent ? *_parent : *this;
    if (const auto [it, succ] =
                _symbol_values.emplace(sym,
                                       unevaluated_value(eval_stack.shared_from_this(), expr));
        !succ) {
        // _symbol_values[sym] = expr;
    }
}

c4::value*
c4::evaluation_stack::eval(const value_map::iterator& it) {
    struct lazy_evaluator final {
        evaluation_stack& stk;
        lazy_value& value;

        c4::value*
        operator()(c4::value&) const {
            return &std::get<c4::value>(value);
        }

        c4::value*
        operator()(const referenced_value far_value) const {
            const auto value_ptr = static_cast<lazy_value*>(far_value);
            return std::visit(lazy_evaluator(stk, *value_ptr), value);
        }

        c4::value*
        operator()(const unevaluated_value& expr) const {
            value = expr.evaluate_and_get();
            return &std::get<c4::value>(value);
        }
    };

    return std::visit(lazy_evaluator(*this, it->second), it->second);
}

c4::value
c4::evaluation_stack::unevaluated_value::evaluate_and_get() const {
    return _expr->evaluate(_context);
}
