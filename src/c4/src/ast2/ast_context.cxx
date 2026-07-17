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
 * src/c4/src/ast2/ast_context --
 *   
 */

#include <list>

#include <c4/ast2/ast_context.hxx>

#include <c4/ast2/ast_node.hxx>
#include <c4/ast2/block.hxx>
#include <c4/ast2/dynamic_call.hxx>
#include <c4/ast2/expression.hxx>
#include <c4/ast2/float_literal.hxx>
#include <c4/ast2/fn_call.hxx>
#include <c4/ast2/integer_literal.hxx>
#include <c4/ast2/let_expression.hxx>
#include <c4/ast2/op_call.hxx>
#include <c4/ast2/string_literal.hxx>
#include <c4/ast2/symbol.hxx>

namespace {
	template<class T, class... Args>
	T*
	build_insert(std::list<std::unique_ptr<c4::ast2::ast_node>>& nodes,
	             Args&&... args) {
		auto arg = std::make_unique<T>(std::forward<Args>(args)...);
		auto* ptr = arg.get();
		nodes.emplace_front(std::move(arg));
		return ptr;
	}
}

c4::ast2::block_args*
c4::ast2::ast_context::build_block_args(const position& position,
                                        std::span<symbol> args) {
	return build_insert<block_args>(_nodes, position, args);
}

c4::ast2::block*
c4::ast2::ast_context::build_block(const position& position,
                                   std::vector<expression*>&& expressions,
                                   block_args* args) {
	return build_insert<block>(_nodes, position, std::move(expressions), args);
}

c4::ast2::dynamic_call*
c4::ast2::ast_context::build_dynamic_call(const position& position,
                                          expression* callee,
                                          std::vector<expression*>&& args) {
	return build_insert<dynamic_call>(_nodes, position, callee, std::move(args));
}

c4::ast2::fn_call*
c4::ast2::ast_context::build_fn_call(const position& position,
                                     const symbol& callee,
                                     std::vector<expression*>&& args) {
	return build_insert<fn_call>(_nodes, position, callee, std::move(args));
}

c4::ast2::expression*
c4::ast2::ast_context::build_expression(const symbol& sym) {
	return build_insert<expression>(_nodes, sym);
}

c4::ast2::expression*
c4::ast2::ast_context::build_expression(const string_literal& str) {
	return build_insert<expression>(_nodes, str);
}

c4::ast2::expression*
c4::ast2::ast_context::build_expression(const float_literal& flt) {
	return build_insert<expression>(_nodes, flt);
}

c4::ast2::expression*
c4::ast2::ast_context::build_expression(const integer_literal& i) {
	return build_insert<expression>(_nodes, i);
}

c4::ast2::expression*
c4::ast2::ast_context::build_expression(block* exp, std::vector<symbol>&& closure) {
	return build_insert<expression>(_nodes, exp, std::move(closure));
}

c4::ast2::expression*
c4::ast2::ast_context::build_expression(dynamic_call* exp, std::vector<symbol>&& closure) {
	return build_insert<expression>(_nodes, exp, std::move(closure));
}

c4::ast2::expression*
c4::ast2::ast_context::build_expression(fn_call* exp, std::vector<symbol>&& closure) {
	return build_insert<expression>(_nodes, exp, std::move(closure));
}

c4::ast2::expression*
c4::ast2::ast_context::build_expression(let_expression* exp, std::vector<symbol>&& closure) {
	return build_insert<expression>(_nodes, exp, std::move(closure));
}

c4::ast2::expression*
c4::ast2::ast_context::build_expression(unary_op_call* exp, std::vector<symbol>&& closure) {
	return build_insert<expression>(_nodes, exp, std::move(closure));
}

c4::ast2::expression*
c4::ast2::ast_context::build_expression(binary_op_call* exp, std::vector<symbol>&& closure) {
	return build_insert<expression>(_nodes, exp, std::move(closure));
}

c4::ast2::let_expression*
c4::ast2::ast_context::build_let_expression(const position& position,
                                            const symbol& sym,
                                            expression* expression) {
	const auto ret = build_insert<let_expression>(_nodes, position, sym, expression);
	_lets.push_back(ret);
	return ret;
}

c4::ast2::unary_op_call*
c4::ast2::ast_context::build_unary_op_call(const position& position,
                                           const symbol& sym,
                                           expression* operand) {
	return build_insert<unary_op_call>(_nodes, position, sym, operand);
}

c4::ast2::binary_op_call*
c4::ast2::ast_context::build_binary_op_call(const position& position,
                                            const symbol& sym,
                                            expression* left,
                                            expression* right) {
	return build_insert<binary_op_call>(_nodes, position, sym, left, right);
}
