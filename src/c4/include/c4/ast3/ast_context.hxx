/* blAST project
 *
 * Copyright (c) 2026 András Bodor <bodand@pm.me>
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
 * Originally created: 2026-03-03.
 *
 * src/c4/include/c4/ast3/ast_context --
 *   
 */
#ifndef BLAST_AST_CONTEXT_HXX
#define BLAST_AST_CONTEXT_HXX

#include <list>
#include <memory>
#include <type_traits>

#include <c4/ast3/ast_node.hxx>

namespace c4::ast3 {
struct primitive;

struct ast_context {
    [[nodiscard]] primitive*
    build_primitive(double value);

    [[nodiscard]] primitive*
    build_primitive(std::int64_t value);

    [[nodiscard]] primitive*
    build_primitive(std::string_view value);

    template <typename T, typename... Args>
        requires std::is_base_of_v<ast_node, T>
    T* build(Args&&... args) {
        auto obj = std::unique_ptr<T>(new T{std::forward<Args>(args)...});
        auto* ptr = obj.get();
        _nodes.emplace_back(std::move(obj));
        return ptr;
    }

private:
    // TODO arena allocator
    std::list<std::unique_ptr<ast_node>> _nodes{};
};
}

#endif
