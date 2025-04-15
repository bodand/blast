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
 * src/c4/include/c4/ast2/expression --
 *   
 */
#ifndef C4_AST2_EXPRESSION_HXX
#define C4_AST2_EXPRESSION_HXX

#include <type_traits>
#include <utility>
#include <variant>

#include <c4/ast2/block.hxx>
#include <c4/ast2/float_literal.hxx>
#include <c4/ast2/fn_call.hxx>
#include <c4/ast2/op_call.hxx>
#include <c4/ast2/integer_literal.hxx>
#include <c4/ast2/let_expression.hxx>
#include <c4/ast2/string_literal.hxx>
#include <c4/ast2/symbol.hxx>
#include <c4/ast2/dynamic_call.hxx>

#include <c4/ast2/tags/source_positioned.hxx>

namespace c4::ast2 {
    struct expression final : ast_node
                              , tags::visitable
                              , tags::source_positioned
                              , tags::evaluation_constness {
        using value_type = std::variant<
            float_literal,
            integer_literal,
            string_literal,
            let_expression*,
            symbol,
            fn_call*,
            dynamic_call*,
            binary_op_call*,
            unary_op_call*,
            block*>;

        explicit
        expression(value_type value,
                   std::vector<symbol>&& closure_over = {});

        expression(const expression&) = delete;

        expression&
        operator=(const expression&) = delete;

        expression(expression&&) noexcept = delete;

        expression&
        operator=(expression&&) noexcept = delete;

        [[nodiscard]] const value_type&
        value() const { return _value; }

        template<class V>
        void
        accept_skip_self(V&& visitor) const {
            std::visit([&v = std::forward<V>(visitor)]<class T>(T&& val) mutable {
                if constexpr (std::is_pointer_v<std::remove_cvref_t<T>>) {
                    std::forward<T>(val)->accept(v);
                }
                else if constexpr (!std::is_pointer_v<std::remove_cvref_t<T>>) {
                    std::forward<T>(val).accept(v);
                }
            }, _value);
        }

        [[nodiscard]] bool
        closure() const noexcept { return !_closure_symbols.empty(); }

        [[nodiscard]] std::span<const symbol>
        closure_symbols() const noexcept { return _closure_symbols; }

        [[nodiscard]] bool
        is_constant_evaluable() const noexcept {
            return std::visit([]<typename T>(const T& x) {
                if constexpr (std::is_pointer_v<T>) {
                    return x->const_evaluable();
                }
                else {
                    return x.const_evaluable();
                }
            }, _value);
        }

        [[nodiscard]] unsigned
        unbound_parameters() const noexcept {
            return std::visit([]<typename T>(const T& x) {
                if constexpr (std::is_pointer_v<T>) {
                    return x->unbound_parameters();
                }
                else {
                    return x.unbound_parameters();
                }
            }, _value);
        }

    private:
        value_type _value;
        std::vector<symbol> _closure_symbols;
    };
}

#endif
