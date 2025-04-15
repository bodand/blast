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
 * src/c4/include/c4/ast_dumper --
 *   
 */
#ifndef C4_AST_DUMPER_HXX
#define C4_AST_DUMPER_HXX

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

#include <ostream>
#include <iomanip>

#include <c4/ast2/visitor/visitor.hxx>

namespace c4 {
    struct ast_dumper final : ast2::visitor<
                ast2::block,
                ast2::block_args,
                ast2::dynamic_call,
                ast2::expression,
                ast2::float_literal,
                ast2::fn_call,
                ast2::integer_literal,
                ast2::let_expression,
                ast2::binary_op_call,
                ast2::unary_op_call,
                ast2::string_literal,
                ast2::symbol,
                ast2::op_symbol
            > {
        void do_visit(const ast2::block_args& obj) override;

        void do_visit(const ast2::block& obj) override;

        void do_visit(const ast2::dynamic_call& obj) override;

        void do_visit(const ast2::expression& obj) override;

        void do_visit(const ast2::fn_call& obj) override;

        void do_visit(const ast2::let_expression& obj) override;

        void do_visit(const ast2::binary_op_call& obj) override;

        void do_visit(const ast2::unary_op_call& obj) override;

        void
        do_visit(const ast2::symbol& obj) override {
            os << obj.name() << "/" << obj.arity();
        }

        void
        do_visit(const ast2::op_symbol& obj) override {
            os << obj.name() << "/" << obj.arity();
        }

        void
        do_visit(const ast2::integer_literal& obj) override {
            os << obj.value();
        }

        void
        do_visit(const ast2::float_literal& obj) override {
            os << obj.value();
        }

        void
        do_visit(const ast2::string_literal& obj) override {
            os << std::quoted(obj.value());
        }

        explicit
        ast_dumper(std::ostream& os)
            : os{os} { }

    private:
        std::ostream& os;
        int depth = 0;
    };
}

#endif
