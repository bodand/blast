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
 * Originally created: 2025-07-07.
 *
 * src/c4/include/c4/ffm/ffm_context --
 *   
 */
#ifndef C4_FFM_CONTEXT_HXX
#define C4_FFM_CONTEXT_HXX

#include <list>
#include <memory>
#include <optional>

#include <c4/diagnostic.hxx>
#include <c4/ffm/ffm_node.hxx>
#include <c4/tags/referable.hxx>

#include "function_call.hxx"

namespace c4::ffm {
    struct context_access;
    struct context_object;
    struct local;
    struct local_ref;
    struct block_argument;
    struct context_type;
    struct function;
    struct function_call;
    struct function_declaration;
    struct function_definition;
    struct root_expression;
    struct value_expression;
    struct literal;
    struct block_literal;
    struct symbol;
    struct unpack;

    struct ffm_context {
        [[nodiscard]] function*
        build_function(function_declaration* decl);

        [[nodiscard]] function*
        build_function(function_definition* def);

        [[nodiscard]] function_call*
        build_function_call(const position& position,
                            function_declaration* fn);

        [[nodiscard]] dynamic_call*
        build_dynamic_call(const position& position,
                           value_expression* expr,
                           bool packed = false);

        [[nodiscard]] function_call*
        build_function_pack(const position& position,
                            function_declaration* fn);

        [[nodiscard]] context_type*
        build_context_type(std::string name,
                           std::vector<ast2::tags::referable*> symbols);

        [[nodiscard]] block_argument*
        build_block_argument(const position& position,
                             std::string_view name,
                             unsigned arity);

        [[nodiscard]] context_access*
        build_context_reference(block_argument* ctx, context_type* type, std::string_view name);

        [[nodiscard]] function_declaration*
        build_function_declaration(const symbol& sym,
                                   context_type* ctx_type = nullptr,
                                   bool known = true,
                                   std::vector<block_argument*>&& args = {});

        [[nodiscard]] function_definition*
        build_function_definition(const function_declaration* decl);

        [[nodiscard]] root_expression*
        build_root_expression(function_call* call);

        [[nodiscard]] root_expression*
        build_root_expression(dynamic_call* dyn);

        [[nodiscard]] root_expression*
        build_root_expression(block_argument* arg);

        [[nodiscard]] root_expression*
        build_root_expression(literal* lit);

        [[nodiscard]] root_expression*
        build_root_expression(block_literal* lit);

        [[nodiscard]] root_expression*
        build_root_expression(unpack* unp);

        [[nodiscard]] root_expression*
        build_root_expression(local* loc);

        [[nodiscard]] value_expression*
        build_value_expression(function_call* call);

        [[nodiscard]] value_expression*
        build_value_expression(dynamic_call* call);

        [[nodiscard]] value_expression*
        build_value_expression(block_argument* arg);

        [[nodiscard]] value_expression*
        build_value_expression(literal* lit);

        [[nodiscard]] value_expression*
        build_value_expression(block_literal* lit);

        [[nodiscard]] value_expression*
        build_value_expression(local_ref* arg);

        [[nodiscard]] value_expression*
        build_value_expression(context_access* arg);

        [[nodiscard]] value_expression*
        build_value_expression(context_object* arg);

        [[nodiscard]] unpack*
        build_unpack(value_expression* expr);

        [[nodiscard]] literal*
        build_literal(int32_t i32);

        [[nodiscard]] literal*
        build_literal(int64_t i64);

        [[nodiscard]] literal*
        build_literal(double d);

        [[nodiscard]] literal*
        build_literal(std::string_view sv);

        [[nodiscard]] block_literal*
        build_block_literal(function_declaration* decl,
                            context_object* context = nullptr);

        [[nodiscard]] local*
        build_local(std::string_view name, value_expression* value);

        [[nodiscard]] local_ref*
        build_local_reference(const local* local);

        [[nodiscard]] context_object*
        build_context_object(context_type* context);

    private:
        // TODO arena allocator
        std::list<std::unique_ptr<ffm_node>> _nodes{};
    };
}

#endif
