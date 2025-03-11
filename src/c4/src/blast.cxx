/* demo project
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
 * Originally created: 2025-02-02.
 *
 * src/blast --
 *   
 */

#include <iomanip>
#include <iostream>

#include <c4/ast.hxx>
#include <c4/block.hxx>
#include <c4/evaluation_stack.hxx>
#include <c4/parser/config.hxx>
#include <c4/parser/expression.hxx>
#include <c4/parser/symbol.hxx>

struct expr_printer {
    using result_type = void;

    void
    operator()(std::monostate) const {
        std::cout << "\n";
    }

    void
    operator()(const c4::ast::let_expression& let) const {
        std::cout << "(let " << let.symbol << " ";
        boost::apply_visitor(*this, let.expression);
        std::cout << ")\n";
    }

    void
    operator()(const c4::ast::op_call& op) const {
        boost::apply_visitor(*this, op.expression);
    }

    void
    operator()(const c4::ast::fundamental_scalar& scalar) const {
        boost::apply_visitor(*this, scalar);
    }

    void
    operator()(const std::int64_t i) const { std::cout << i; }

    void
    operator()(const double d) const { std::cout << d; }

    void
    operator()(const std::string& s) const { std::cout << std::quoted(s); }

    void
    operator()(const c4::ast::symbol& s) const { std::cout << s; }

    void
    operator()(const c4::ast::block_expression& block) const {
        std::cout << "(lambda ";
        if (!block.parameters.symbols.empty()) {
            std::cout << "[";
            for (const auto& sym: block.parameters.symbols)
                std::cout << sym << " ";
            std::cout << "\b] ";
        }

        for (const auto& expr: block.exprs)
            boost::apply_visitor(*this, expr);
        std::cout << ")";
    }

    void
    operator()(const c4::ast::fn_call& fn) const {
        std::cout << "(";
        boost::apply_visitor(*this, fn.callee);
        std::cout << " ";
        for (const auto& expr: fn.args) {
            boost::apply_visitor(*this, expr);
            std::cout << " ";
        }
        std::cout << "\b)";
    }

    void
    operator()(const c4::ast::call_expr& expr) const {
        boost::apply_visitor(*this, expr.expr);
    }

    void
    operator()(const c4::ast::expression& expr) const {
        boost::apply_visitor(*this, expr);
    }
};


int
main() {
    namespace c4p = ::c4::parser;
    namespace c4a = ::c4::ast;

    std::vector<c4a::expression> expressions;

    std::string buf = R"__(
let a/0 41
let printinc/1 { |x| { print add 1 x } }

&printinc a/0
)__";

    auto begin = buf.cbegin();
    const auto end = buf.cend();

    using boost::spirit::x3::with;

    c4p::position_cache position_cache{begin, end};
    c4p::error_handler eh{begin, end, std::cerr, "<string>"};
    c4a::symbol_scope global_scope;
    const auto parser =
            with<c4p::position_cache_tag>(std::ref(position_cache))[
                with<c4p::error_handler_tag>(std::ref(eh))[
                    with<c4p::symbol_scope_tag>(std::ref(global_scope))[
                        *c4p::expression()
                    ]]];

    global_scope.define(c4a::symbol("print", 1));
    global_scope.define(c4a::symbol("add", 2));

    c4::evaluation_stack stack;
    stack.set(c4::symbol("print", 1),
              std::unique_ptr<c4::block, c4::block_deleter>(
                  new c4::native_block([](auto& stack) -> c4::value {
                      auto val = stack.value_of(c4::symbol("$0", 0));
                      std::cout << **val << "\n";
                      return std::move(**val);
                  })));
    stack.set(c4::symbol("add", 2),
              std::unique_ptr<c4::block, c4::block_deleter>(
                  new c4::native_block([](auto& stack) -> c4::value {
                      auto val0 = *stack.value_of(c4::symbol("$0", 0));
                      auto val1 = *stack.value_of(c4::symbol("$1", 0));
                      return val0->coerce_to_int() + val1->coerce_to_int();
                  })));

    try {
        bool r = phrase_parse(begin, end,
                              parser,
                              boost::spirit::x3::ascii::space,
                              expressions);

        if (r && begin == end) {
            std::cout << "-------------------------\n";
            std::cout << "Parsing succeeded: \n";
            std::cout << buf << "-> \n";
            for (const auto& expr: expressions) {
                boost::apply_visitor(expr_printer{}, expr);
            }
            std::cout << "\noutput: \n";
            for (const auto& expr: expressions) {
                expr.evaluate(stack);
            }
            std::cout << "\n-------------------------\n";
        }
        else {
            std::cout << "-------------------------\n";
            std::cout << "Parsing failed\n";
            std::cout << "-------------------------\n";
        }
    }
    catch (std::runtime_error& x) {
        std::cerr << x.what() << "\n";
    }
}
