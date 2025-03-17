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
 * src/c4/include/c4/evaluation_stack --
 *   
 */
#ifndef EVALUATION_STACK_HXX
#define EVALUATION_STACK_HXX

#include <cassert>
#include <unordered_map>
#include <unordered_set>
#include <optional>
#include <utility>
#include <variant>
#include <memory>

#include <c4/value.hxx>
#include <c4/symbol.hxx>

namespace c4 {
    namespace ast {
        struct expression;
    }

    struct evaluation_stack final : std::enable_shared_from_this<evaluation_stack> {
        evaluation_stack() = default;

        explicit
        evaluation_stack(const std::shared_ptr<evaluation_stack>& parent)
            : _parent{parent} { }

        evaluation_stack(const evaluation_stack&) = delete;

        evaluation_stack&
        operator=(const evaluation_stack&) = delete;

        std::shared_ptr<evaluation_stack>
        push() {
            return std::make_shared<evaluation_stack>(shared_from_this());
        }

        void
        merge(evaluation_stack& other);

        std::optional<value*>
        value_of(const symbol& sym) {
            auto searched_stack = this;
            do {
                if (const auto it = searched_stack->_symbol_values.find(sym);
                    it != searched_stack->_symbol_values.end()) {
                    return eval(it);
                }
                searched_stack = searched_stack->_parent.get();
            } while (searched_stack);
            return std::nullopt;
        }

        void
        set(const symbol& sym, value&& val);

        void
        set(const symbol& sym, const ast::expression* expr);

    private:
        struct unevaluated_value {
            const std::shared_ptr<evaluation_stack> _context;
            const ast::expression* _expr;

            unevaluated_value(std::shared_ptr<evaluation_stack>&& context, const ast::expression* expr)
                : _context{std::move(context)}
                , _expr{expr} { }

            [[nodiscard]] value
            evaluate_and_get() const;
        };

        using referenced_value = void*;
        using lazy_value = std::variant<unevaluated_value, value, referenced_value>;
        using value_map = std::unordered_map<symbol, lazy_value>;

        value*
        eval(const value_map::iterator& it);

        std::shared_ptr<evaluation_stack> _parent{};
        value_map _symbol_values{};
    };
}

#endif
