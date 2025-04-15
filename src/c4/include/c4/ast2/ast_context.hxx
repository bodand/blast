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
 * Originally created: 2025-04-04.
 *
 * src/c4/include/c4/ast2/ast_context --
 *   ast_context handles the lifetime of ast_nodes. It gives out pointers to the
 *   given elements which stay valid until the end of the context's lifetime.
 */
#ifndef C4_AST2_AST_CONTEXT_HXX
#define C4_AST2_AST_CONTEXT_HXX

#include <deque>
#include <memory>
#include <span>

#include <c4/ast2/ast_node.hxx>

#include <c4/ast2/float_literal.hxx>
#include <c4/ast2/integer_literal.hxx>
#include <c4/ast2/string_literal.hxx>
#include <c4/ast2/symbol.hxx>

namespace c4::ast2 {
    struct block_argument;
    struct block_args;
    struct block;
    struct dynamic_call;
    struct expression;
    struct fn_call;
    struct let_expression;
    struct unary_op_call;
    struct binary_op_call;

    struct ast_context final {
        pseudo_node*
        build_pseudo_node(std::string_view);

        block_args*
        build_block_args(const position& position,
                         std::string_view file_source,
                         std::size_t length,
                         std::span<symbol> args);

        block*
        build_block(const position& position,
                    std::string_view file_source,
                    std::size_t length,
                    std::vector<expression*>&& expressions,
                    block_args* args = nullptr);

        dynamic_call*
        build_dynamic_call(const position& position,
                           std::string_view file_source,
                           std::size_t length,
                           expression* callee,
                           std::vector<expression*>&& args);

        fn_call*
        build_fn_call(const position& position,
                      std::string_view file_source,
                      std::size_t length,
                      const symbol& callee,
                      std::vector<expression*>&& args);

        expression*
        build_expression(const symbol& sym);

        expression*
        build_expression(const string_literal& str);

        expression*
        build_expression(const float_literal& flt);

        expression*
        build_expression(const integer_literal& i);

        expression*
        build_expression(block* exp, std::vector<symbol>&& closure = {});

        expression*
        build_expression(dynamic_call* exp, std::vector<symbol>&& closure = {});

        expression*
        build_expression(fn_call* exp, std::vector<symbol>&& closure = {});

        expression*
        build_expression(let_expression* exp, std::vector<symbol>&& closure = {});

        expression*
        build_expression(unary_op_call* exp, std::vector<symbol>&& closure = {});

        expression*
        build_expression(binary_op_call* exp, std::vector<symbol>&& closure = {});

        let_expression*
        build_let_expression(const position& position,
                             std::string_view file_source,
                             std::size_t length,
                             const symbol& sym,
                             expression* expression);

        unary_op_call*
        build_unary_op_call(const position& position,
                            std::string_view file_source,
                            std::size_t length,
                            const symbol& sym,
                            expression* operand);

        binary_op_call*
        build_binary_op_call(const position& position,
                             std::string_view file_source,
                             std::size_t length,
                             const symbol& sym,
                             expression* left,
                             expression* right);

    private:
        std::list<std::unique_ptr<ast_node>> _nodes;
    };
}

#endif
