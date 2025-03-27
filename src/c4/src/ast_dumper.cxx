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
 * src/c4/src/ast_dumper --
 *   
 */

#include <c4/ast_dumper.hxx>
#include <uni_algo/ranges.h>

void
c4::ast_dumper::do_visit(const ast2::block_args& obj) {
    os << "[";
    auto args = obj.args();
    if (args.empty()) {
        os << "]";
        return;
    }

    for (const auto& expr: args | std::views::take(args.size() - 1)) {
        expr.accept(*this);
        os << " ";
    }
    args.back().accept(*this);
    os << "]";
}

void
c4::ast_dumper::do_visit(const ast2::block& obj) {
    os << "(lambda ";
    if (obj.args()) {
        obj.args()->accept(*this);
        os << " ";
    }
    auto expressions = obj.expressions();
    if (expressions.empty()) {
        os << ")";
        return;
    }

    for (const auto& expr: expressions | std::views::take(expressions.size() - 1)) {
        expr.accept(*this);
        os << " ";
    }
    expressions.back().accept(*this);
    os << ")";
}

void
c4::ast_dumper::do_visit(const ast2::dynamic_call& obj) {
    os << "(";
    obj.callee().accept(*this);
    os << " ";
    auto expressions = obj.args();
    if (expressions.empty()) {
        os << ")";
        return;
    }

    for (const auto& expr: expressions | std::views::take(expressions.size() - 1)) {
        expr.accept(*this);
        os << " ";
    }
    expressions.back().accept(*this);
    os << ")";
}

void
c4::ast_dumper::do_visit(const ast2::expression& obj) {
    if (!obj.closure()) return obj.accept_skip_self(*this);
    os << "(scope [";
    auto syms = obj.closure_symbols();
    for (const auto& expr: syms | std::views::take(syms.size() - 1)) {
        expr.accept(*this);
        os << " ";
    }
    syms.back().accept(*this);
    os << "] ";

    obj.accept_skip_self(*this);
    os << ")";
}

void
c4::ast_dumper::do_visit(const ast2::fn_call& obj) {
    os << "(";
    obj.sym().accept(*this);
    os << " ";

    auto expressions = obj.args();
    if (expressions.empty()) {
        os << ")";
        return;
    }

    for (const auto& expr: expressions | std::views::take(expressions.size() - 1)) {
        expr.accept(*this);
        os << " ";
    }
    expressions.back().accept(*this);
    os << ")";
}

void
c4::ast_dumper::do_visit(const ast2::let_expression& obj) {
    os << "(let ";
    obj.symbol().accept(*this);
    os << " ";
    obj.value().accept(*this);
    os << ")";
}

void
c4::ast_dumper::do_visit(const ast2::binary_op_call& obj) {
    os << "(";
    obj.op().accept(*this);
    os << " ";
    obj.left().accept(*this);
    os << " ";
    obj.right().accept(*this);
    os << ")";
}

void
c4::ast_dumper::do_visit(const ast2::unary_op_call& obj) {
    os << "(";
    obj.op().accept(*this);
    os << " ";
    obj.operand().accept(*this);
    os << ")";
}
