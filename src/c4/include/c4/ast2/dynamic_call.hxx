/* blAST project
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
 * src/c4/include/c4/ast2/dynamic_call --
 *   
 */
#ifndef C4_AST2_DYNAMIC_CALL_HXX
#define C4_AST2_DYNAMIC_CALL_HXX

#include <span>

#include <c4/ast2/tags/visitable.hxx>
#include <c4/ast2/tags/source_positioned.hxx>
#include <c4/ast2/tags/evaluation_constness.hxx>

namespace c4::ast2 {
    struct expression;

    struct dynamic_call final : ast_node
                                , tags::visitable
                                , tags::source_positioned
                                , tags::dynamic_node {
        dynamic_call(const c4::position& position,
                     std::string_view file_source,
                     std::size_t length,
                     expression* callee,
                     std::vector<expression*>&& args);

        dynamic_call(const dynamic_call& cp) = delete;

        dynamic_call&
        operator=(const dynamic_call& cp) = delete;

        dynamic_call(dynamic_call&& other) noexcept = delete;

        dynamic_call&
        operator=(dynamic_call&& other) noexcept = delete;

        [[nodiscard]] const expression*
        callee() const;

        [[nodiscard]] std::span<const expression* const>
        args() const;

    private:
        expression* _callee;
        std::vector<expression*> _args;
    };
}

#endif
