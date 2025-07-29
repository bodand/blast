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
 * src/c4/include/c4/ast2/op_call --
 *   
 */
#ifndef C4_AST2_OP_CALL_HXX
#define C4_AST2_OP_CALL_HXX

#include <c4/ast2/ast_node.hxx>
#include <c4/ast2/symbol.hxx>

#include <c4/tags/source_positioned.hxx>
#include <c4/tags/visitable.hxx>

namespace c4::ast2 {
    struct expression;

    struct binary_op_call final : ast_node
                                  , tags::visitable
                                  , tags::source_positioned
                                  , tags::dynamic_node {
        binary_op_call(const c4::position& position,
                       const symbol& op,
                       expression* left,
                       expression* right);

        binary_op_call(const binary_op_call& cp) = delete;

        binary_op_call&
        operator=(const binary_op_call& cp) = delete;

        binary_op_call(binary_op_call&& other) noexcept = delete;

        binary_op_call&
        operator=(binary_op_call&& other) noexcept = delete;

        [[nodiscard]] symbol
        op() const noexcept { return _op; }

        [[nodiscard]] const expression&
        left() const noexcept;

        [[nodiscard]] const expression&
        right() const noexcept;

        [[nodiscard]] unsigned
        unbound_parameters() const noexcept { return 0; }

    private:
        symbol _op;
        expression* _left;
        expression* _right;
    };

    struct unary_op_call final : ast_node
                                 , tags::visitable
                                 , tags::source_positioned
                                 , tags::dynamic_node {
        unary_op_call(const c4::position& position,
                      const symbol& op,
                      expression* operand);

        unary_op_call(const unary_op_call& cp) = delete;

        unary_op_call&
        operator=(const unary_op_call& cp) = delete;

        unary_op_call(unary_op_call&&) noexcept = delete;

        unary_op_call&
        operator=(unary_op_call&&) noexcept = delete;

        [[nodiscard]] symbol
        op() const noexcept { return _op; }

        [[nodiscard]] const expression&
        operand() const noexcept;

        [[nodiscard]] unsigned
        unbound_parameters() const noexcept { return 0; }

    private:
        symbol _op;
        expression* _operand;
    };
}

#endif
