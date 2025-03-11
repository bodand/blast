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
 * src/c4/src/ast/symbol_scope --
 *   
 */

#include <tuple>
#include <algorithm>
#include <ranges>

#include <c4/ast/symbol_scope.hxx>

void
c4::ast::symbol_scope::define(symbol symbol, bool is_mutable, int precedence) {
    auto [it, success] = _definitions.try_emplace(symbol,
                                                  symbol, is_mutable, precedence);
    if (!success) throw symbol_redefinition(symbol);
}

void
c4::ast::symbol_scope::load_parser(x3::symbols<symbol>& symbols) {
    for (const auto& symbol: _definitions | std::views::keys)
        std::ignore = symbols.add(symbol.name, symbol);
    if (_parent) _parent->load_parser(symbols);
}

c4::ast::symbol_scope*
c4::ast::symbol_scope::new_scope() {
    _children_scopes.emplace_front(this);
    return &_children_scopes.front();
}

c4::ast::symbol_definition*
c4::ast::symbol_scope::find_symbol(const symbol& symbol) {
    if (const auto it = _definitions.find(symbol);
        it != _definitions.end()) {
        return &it->second;
    }

    if (!_parent) return nullptr;
    return _parent->find_symbol(symbol);
}
