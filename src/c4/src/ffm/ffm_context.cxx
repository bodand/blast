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
 * src/c4/src/ffm/ffm_context --
 *   
 */

#include <c4/ast2/block.hxx>
#include <c4/ffm/ffm_context.hxx>

#include <c4/ffm/block_argument.hxx>
#include <c4/ffm/expression.hxx>
#include <c4/ffm/ffm_node.hxx>
#include <c4/ffm/function.hxx>
#include <c4/ffm/function_call.hxx>
#include <c4/ffm/function_declaration.hxx>
#include <c4/ffm/function_definition.hxx>
#include <c4/ffm/function_pack.hxx>
#include <c4/ffm/literal.hxx>
#include <c4/ffm/local.hxx>
#include <c4/ffm/symbol.hxx>
#include <c4/ffm/unpack.hxx>

namespace {
    template<class T, class... Args>
    T*
    build_insert(std::list<std::unique_ptr<c4::ffm::ffm_node>>& nodes,
                 Args&&... args) {
        auto arg = std::make_unique<T>(std::forward<Args>(args)...);
        auto* ptr = arg.get();
        nodes.emplace_front(std::move(arg));
        return ptr;
    }
}

c4::ffm::function*
c4::ffm::ffm_context::build_function(function_declaration* const decl) {
    return build_insert<function>(_nodes, decl);
}

c4::ffm::function*
c4::ffm::ffm_context::build_function(function_definition* const def) {
    return build_insert<function>(_nodes, def);
}

c4::ffm::function_call*
c4::ffm::ffm_context::build_function_call(const position& position, function_declaration* const fn) {
    return build_insert<function_call>(_nodes, position, fn);
}

c4::ffm::function_pack*
c4::ffm::ffm_context::build_function_pack(const position& position, function_declaration* const fn) {
    return build_insert<function_pack>(_nodes, position, fn);
}

c4::ffm::context_type*
c4::ffm::ffm_context::build_context_type(std::string name,
                                         std::vector<ast2::tags::referable*> symbols) {
    return build_insert<context_type>(_nodes, std::move(name), std::move(symbols));
}

c4::ffm::block_argument*
c4::ffm::ffm_context::build_block_argument(const position& position,
                                           const std::string_view name,
                                           const unsigned arity) {
    return build_insert<block_argument>(_nodes, position, name, arity);
}

c4::ffm::context_access*
c4::ffm::ffm_context::build_context_reference(block_argument* ctx, context_type* type, std::string_view name) {
    return build_insert<context_access>(_nodes, ctx, type, name);
}

c4::ffm::function_declaration*
c4::ffm::ffm_context::build_function_declaration(const symbol& sym,
                                                 context_type* const ctx_type,
                                                 const bool known,
                                                 std::vector<block_argument*>&& args) {
    return build_insert<function_declaration>(_nodes, sym, ctx_type, known, std::move(args));
}

c4::ffm::function_definition*
c4::ffm::ffm_context::build_function_definition(const function_declaration* const decl) {
    return build_insert<function_definition>(_nodes, decl);
}

c4::ffm::root_expression*
c4::ffm::ffm_context::build_root_expression(function_call* const call) {
    return build_insert<root_expression>(_nodes, call);
}

c4::ffm::root_expression*
c4::ffm::ffm_context::build_root_expression(block_argument* const arg) {
    return build_insert<root_expression>(_nodes, arg);
}

c4::ffm::root_expression*
c4::ffm::ffm_context::build_root_expression(literal* const lit) {
    return build_insert<root_expression>(_nodes, lit);
}

c4::ffm::root_expression*
c4::ffm::ffm_context::build_root_expression(unpack* const unp) {
    return build_insert<root_expression>(_nodes, unp);
}

c4::ffm::root_expression*
c4::ffm::ffm_context::build_root_expression(local* const loc) {
    return build_insert<root_expression>(_nodes, loc);
}

c4::ffm::value_expression*
c4::ffm::ffm_context::build_value_expression(function_pack* const call) {
    return build_insert<value_expression>(_nodes, call);
}

c4::ffm::value_expression*
c4::ffm::ffm_context::build_value_expression(block_argument* const arg) {
    return build_insert<value_expression>(_nodes, arg);
}

c4::ffm::value_expression*
c4::ffm::ffm_context::build_value_expression(literal* const lit) {
    return build_insert<value_expression>(_nodes, lit);
}

c4::ffm::value_expression*
c4::ffm::ffm_context::build_value_expression(local_ref* const arg) {
    return build_insert<value_expression>(_nodes, arg);
}

c4::ffm::value_expression*
c4::ffm::ffm_context::build_value_expression(context_access* arg) {
    return build_insert<value_expression>(_nodes, arg);
}

c4::ffm::unpack*
c4::ffm::ffm_context::build_unpack(value_expression* const expr) {
    return build_insert<unpack>(_nodes, expr);
}

c4::ffm::literal*
c4::ffm::ffm_context::build_literal(const int32_t i32) {
    return build_insert<literal>(_nodes, i32);
}

c4::ffm::literal*
c4::ffm::ffm_context::build_literal(const int64_t i64) {
    return build_insert<literal>(_nodes, i64);
}

c4::ffm::literal*
c4::ffm::ffm_context::build_literal(const double d) {
    return build_insert<literal>(_nodes, d);
}

c4::ffm::literal*
c4::ffm::ffm_context::build_literal(const std::string_view sv) {
    return build_insert<literal>(_nodes, sv);
}

c4::ffm::local*
c4::ffm::ffm_context::build_local(const std::string_view name,
                                  value_expression* const value) {
    return build_insert<local>(_nodes, name, value);
}

c4::ffm::local_ref*
c4::ffm::ffm_context::build_local_reference(const local* const local) {
    return build_insert<local_ref>(_nodes, local);
}
