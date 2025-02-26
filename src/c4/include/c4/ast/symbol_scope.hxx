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
 * src/c4/include/c4/ast/symbol_scope --
 *   
 */
#ifndef AST_SYMBOL_SCOPE_HXX
#define AST_SYMBOL_SCOPE_HXX

#include <c4/ast/symbol.hxx>
#include <unordered_map>
#include <forward_list>
#include <format>

namespace c4::ast {
    struct symbol_redefinition final : std::runtime_error {
        explicit
        symbol_redefinition(const symbol& symbol)
            : runtime_error(std::format(
                "attempted redefinition of symbol {}/{} when not marked as mutable at original definition",
                symbol.name, symbol.arity))
            , symbol(symbol) { }

        symbol symbol;
    };

    struct symbol_definition {
        symbol symbol;
        bool is_mutable;
        int precedence;
    };

    struct symbol_scope {
        symbol_scope() = default;

        explicit
        symbol_scope(symbol_scope* parent)
            : _parent{parent} { }

        symbol_scope(const symbol_scope& other)
            : _parent(other._parent)
            , _children_scopes(other._children_scopes)
            , _definitions(other._definitions) { }

        symbol_scope(symbol_scope&& other) noexcept
            : _parent(other._parent)
            , _children_scopes(std::move(other._children_scopes))
            , _definitions(std::move(other._definitions)) { }

        symbol_scope&
        operator=(const symbol_scope& other) {
            if (this == &other)
                return *this;
            _parent = other._parent;
            _children_scopes = other._children_scopes;
            _definitions = other._definitions;
            return *this;
        }

        symbol_scope&
        operator=(symbol_scope&& other) noexcept {
            if (this == &other)
                return *this;
            _parent = other._parent;
            _children_scopes = std::move(other._children_scopes);
            _definitions = std::move(other._definitions);
            return *this;
        }

        [[nodiscard]] symbol_scope*
        new_scope();

        [[nodiscard]] symbol_definition*
        find_symbol(const symbol& symbol);

        void
        define(symbol symbol, bool is_mutable = false, int precedence = 0);

        [[nodiscard]] symbol_scope&
        parent() const { return *_parent; }

    private:
        symbol_scope* _parent{};
        std::forward_list<symbol_scope> _children_scopes{};
        std::unordered_map<symbol, symbol_definition> _definitions{};
    };
}

#endif
