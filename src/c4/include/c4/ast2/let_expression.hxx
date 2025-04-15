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
 * src/c4/include/c4/ast2/let_expression --
 *   
 */
#ifndef C4_AST2_LET_EXPRESSION_HXX
#define C4_AST2_LET_EXPRESSION_HXX

#include <c4/ast2/ast_node.hxx>
#include <c4/ast2/symbol.hxx>

#include <c4/ast2/tags/referable.hxx>
#include <c4/ast2/tags/source_positioned.hxx>
#include <c4/ast2/tags/visitable.hxx>

#include <string_view>

namespace c4::ast2 {
    struct expression;

    struct let_expression final : tags::referable
                                  , ast_node
                                  , tags::visitable
                                  , tags::source_positioned
                                  , tags::evaluation_constness {
        let_expression(const c4::position& position,
                       std::size_t length,
                       symbol symbol,
                       expression* expr);

        let_expression(const let_expression& cp) = delete;

        let_expression&
        operator=(const let_expression& cp) = delete;

        let_expression(let_expression&&) noexcept = delete;

        let_expression&
        operator=(let_expression&&) noexcept = delete;

        [[nodiscard]] symbol
        symbol() const { return _symbol; }

        [[nodiscard]] unsigned
        symbol_arity() const { return _symbol.arity(); }

        [[nodiscard]] const expression&
        value() const;

        [[nodiscard]] std::string
        mangled_name() const { return _symbol.mangle(); }

        [[nodiscard]] bool
        is_constant_evaluable() const noexcept;

        std::string_view
        name() override { return _symbol.name(); }

    private:
        struct symbol _symbol;
        expression* _value;
    };
}

#endif
