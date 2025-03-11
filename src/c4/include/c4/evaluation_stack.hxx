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

#include <unordered_map>
#include <optional>
#include <variant>

#include <c4/value.hxx>
#include <c4/symbol.hxx>

namespace c4 {
    namespace ast {
        struct expression;
    }

    struct evaluation_stack final {
        evaluation_stack() = default;

        evaluation_stack(const evaluation_stack&) = delete;

        evaluation_stack&
        operator=(const evaluation_stack&) = delete;

        evaluation_stack
        push() {
            return evaluation_stack(this);
        }

        std::optional<value*>
        value_of(const symbol& sym) {
            if (const auto it = _symbol_values.find(sym);
                it != _symbol_values.end()) {
                return eval(it);
            }
            if (!_parent) return std::nullopt;
            return _parent->value_of(sym);
        }

        void
        set(const symbol& sym, value&& val);

        void
        set(const symbol& sym,const ast::expression* expr);

    private:
        using value_map = std::unordered_map<symbol,
                                             std::variant<const ast::expression*, value>>;

        std::optional<value*>
        eval(value_map::iterator it);

        explicit
        evaluation_stack(evaluation_stack* parent)
            : _parent{parent} { }

        evaluation_stack* _parent{};
        value_map _symbol_values{};
    };
}

#endif
