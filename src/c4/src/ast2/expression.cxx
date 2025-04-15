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
 * src/c4/src/ast2/expression --
 *   
 */

#include <c4/ast2/float_literal.hxx>
#include <c4/ast2/integer_literal.hxx>
#include <c4/ast2/let_expression.hxx>
#include <c4/ast2/string_literal.hxx>
#include <c4/ast2/symbol.hxx>
#include <c4/ast2/expression.hxx>

#include <algorithm>
#include <ranges>
#include <memory>

#include "c4/ast2/visitor/visitor.hxx"

namespace {
    struct value_extractor final {
        const c4::ast2::tags::source_positioned&
        operator()(const c4::ast2::tags::source_positioned& sp) const noexcept {
            return sp;
        }

        const c4::ast2::tags::source_positioned&
        operator()(c4::ast2::tags::source_positioned* const& sp) const noexcept {
            return *sp;
        }
    };

    void
    uniqify_symbols(std::vector<c4::ast2::symbol>& symbols) {
        if (symbols.empty()) return;
        std::ranges::sort(symbols);
        const auto [dup_begin, dup_end] = std::ranges::unique(symbols);
        symbols.erase(dup_begin, dup_end);
    }

    struct recursive_closure_collector_visitor final : c4::ast2::visitor<
                c4::ast2::expression,
                c4::ast2::let_expression,
                c4::ast2::block,
                c4::ast2::fn_call,
                c4::ast2::dynamic_call,
                c4::ast2::binary_op_call,
                c4::ast2::unary_op_call
            > {
        explicit
        recursive_closure_collector_visitor(std::vector<c4::ast2::symbol>& symbols)
            : symbols(symbols) { }

        void
        do_visit(const c4::ast2::expression& obj) override {
            symbols.append_range(obj.closure_symbols());
        }

        void
        do_visit(const c4::ast2::let_expression& obj) override {
            obj.value().accept(*this);
        }

        void
        do_visit(const c4::ast2::block& obj) override {
            auto block_symols = obj.effective_context_symbols();
            symbols.append_range(std::move(block_symols));
        }

        void
        do_visit(const c4::ast2::fn_call& obj) override {
            for (const auto& arg : obj.args()) arg->accept(*this);
        }

        void
        do_visit(const c4::ast2::dynamic_call& obj) override {
            obj.callee()->accept(*this);
            for (const auto& arg : obj.args()) arg->accept(*this);
        }

        void
        do_visit(const c4::ast2::binary_op_call& obj) override {
            obj.left().accept(*this);
            obj.right().accept(*this);
        }

        void
        do_visit(const c4::ast2::unary_op_call& obj) override {
            obj.operand().accept(*this);
        }

        std::vector<c4::ast2::symbol>& symbols;
    };
}

c4::ast2::expression::expression(value_type value,
                                 std::vector<symbol>&& closure_over)
    : source_positioned{std::visit(value_extractor{}, value)}
    , _value{std::move(value)}
    , _closure_symbols{std::move(closure_over)} {
    std::visit([&sym = _closure_symbols]<class T>(T&& val) mutable {
        recursive_closure_collector_visitor v{sym};
        if constexpr (std::is_pointer_v<std::remove_cvref_t<T>>) {
            std::forward<T>(val)->accept(v);
        }
        else {
            std::forward<T>(val).accept(v);
        }
    }, _value);
    uniqify_symbols(_closure_symbols);
}

// TODO remove
// c4::ast2::expression::expression(const c4::position& position,
//                                  const std::string_view file_source,
//                                  const std::size_t length,
//                                  value_type value,
//                                  const std::span<symbol> closure_over)
//     : source_positioned{position, file_source, length}
//       , _value{std::move(value)}
//       , _closure_symbols{closure_over.begin(), closure_over.end()} {
//     std::visit([&sym = _closure_symbols]<class T>(T&& val) mutable {
//         recursive_closure_collector_visitor v{sym};
//         std::forward<T>(val).accept(v);
//     }, _value);
//     uniqify_symbols(_closure_symbols);
// }
