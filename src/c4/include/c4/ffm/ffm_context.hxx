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

namespace c4::ffm {
    struct block_argument;
    struct context_type;
    struct function;
    struct function_call;
    struct function_declaration;
    struct function_definition;
    struct function_pack;
    struct root_expression;
    struct value_expression;
    struct symbol;

    struct ffm_context {
        function*
        build_function(function_declaration* decl);

        function*
        build_function(function_definition* def);

        function_call*
        build_function_call(const position& position,
                            function_declaration* fn);

        function_pack*
        build_function_pack(const position& position,
                            function_declaration* fn);

        context_type*
        build_context_type(std::string name,
                           std::vector<block_argument*> symbols);

        block_argument*
        build_block_argument(const position& position,
                             std::string_view name,
                             unsigned arity);

        function_declaration*
        build_function_declaration(const symbol& sym,
                                   context_type* ctx_type = nullptr,
                                   bool known = true,
                                   std::vector<block_argument*>&& args = {});

        function_definition*
        build_function_definition(const function_declaration* decl);

        root_expression*
        build_root_expression(function_call* call);

        value_expression*
        build_value_expression(function_pack* call);

    private:
        // TODO arena allocator
        std::list<std::unique_ptr<ffm_node>> _nodes{};
    };
}

#endif
