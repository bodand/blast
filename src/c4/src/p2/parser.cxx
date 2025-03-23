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
 * Originally created: 2025-03-03.
 *
 * src/c4/src/p2/parser --
 *   
 */

#include <c4/p2/parser.hxx>
#include <c4/p2/lex/tokens.hxx>

#include <libassert/assert.hpp>

namespace {
    struct token_ignorer {
        bool& valid;

        bool
        operator()(const c4::p2::tokens::whitespace&) const { return true; }

        bool
        operator()(const c4::p2::tokens::comment&) const { return true; }

        bool
        operator()(const c4::p2::tokens::unknown& unk) const {
            valid = false;
            // format position before passing it to source_diagnostic to escape
            // the likely unprintable characters:
            auto pos = unk.token_position();
            const auto debug_line = fmt::format("{:?}", pos.line);
            pos.line = debug_line;
            fmt::print("{}\n", c4::source_diagnostic::error(fmt::format("unknown characters found: {:?}", unk.value()),
                                                            unk.source_name(),
                                                            pos,
                                                            unk.value().length()));
            fmt::print("{}\n", c4::source_diagnostic::note(
                           "line is escaped because of possibly unprintable characters",
                           unk.source_name(),
                           pos,
                           unk.value().length()));
            return true;
        }

        bool
        operator()(const auto&) const { return false; }
    };

    template<class... Failures>
    [[noreturn]] void
    report_failure(Failures&&... failures) {
        DEBUG_ASSERT((!failures.has_value() && ...),
                     "no failures to report");
        (fmt::print("{}\n", failures.error()), ...);
        throw c4::p2::bad_token_error{};
    }
}

c4::ast2::integer_literal
c4::p2::parser::parse_integer_literal() {
    const auto literal = expect_token<tokens::integer_literal>();
    if (literal) {
        next_relevant();
        return {
            literal->token_position(),
            literal->source_name(),
            literal->value().size(),
            literal->int_value()
        };
    }

    report_failure(literal);
}

c4::ast2::float_literal
c4::p2::parser::parse_float_literal() {
    const auto literal = expect_token<tokens::float_literal>();
    if (literal) {
        next_relevant();
        return {
            literal->token_position(),
            literal->source_name(),
            literal->value().size(),
            literal->float_value()
        };
    }

    report_failure(literal);
}

c4::ast2::string_literal
c4::p2::parser::parse_string_literal() {
    const auto literal = expect_token<tokens::string_literal>();
    if (literal) {
        next_relevant();
        return {
            literal->token_position(),
            literal->source_name(),
            literal->value().size(),
            literal->string_value()
        };
    }

    report_failure(literal);
}

c4::ast2::symbol
c4::p2::parser::parse_symbol() {
    const auto symbol = expect_token<tokens::symbol>();
    if (symbol) {
        next_relevant();
        return {
            symbol->token_position(),
            symbol->source_name(),
            symbol->value().size(),
            symbol->name(),
            symbol->arity()
        };
    }

    const auto bare_symbol = expect_token<tokens::bare_symbol>();
    if (bare_symbol) {
        next_relevant();
        return {
            bare_symbol->token_position(),
            bare_symbol->source_name(),
            bare_symbol->value().size(),
            bare_symbol->name(),
            bare_symbol->arity()
        };
    }

    report_failure(symbol, bare_symbol);
}

c4::ast2::symbol
c4::p2::parser::parse_bare_symbol() {
    const auto bare_symbol = expect_token<tokens::bare_symbol>();
    if (bare_symbol) {
        next_relevant();
        return {
            bare_symbol->token_position(),
            bare_symbol->source_name(),
            bare_symbol->value().size(),
            bare_symbol->name(),
            bare_symbol->arity()
        };
    }

    report_failure(bare_symbol);
}

c4::ast2::expression
c4::p2::parser::parse_expression() {
    const auto let = expect_token<tokens::let>();
    if (let) {
        next_relevant();
        return ast2::expression(parse_let_expression());
    }

    try {
        auto lhs = parse_final_expression();
        const auto op = parse_operator_precedence(std::move(lhs), 0);
        return op;
    }
    catch (...) {
        // hijack exception, and report our own failures (and throw bad_token)
        report_failure(let);
    }
}

c4::ast2::let_expression
c4::p2::parser::parse_let_expression() {
    if (const auto let = expect_token<tokens::let>();
        !let)
        report_failure(let);
    next_relevant();

    // Symbol declaration happens immediately after parsing the symbol: this is
    // required to allow recursion. If symbol was declared at the end of the
    // let expression, the expression parsing after this could not refer to it
    const auto symbol = parse_symbol();
    declare_symbol(symbol.name(), symbol.arity());

    auto expr = parse_expression();
    return {
        symbol.position(),
        symbol.file_source(),
        symbol.length(),
        symbol,
        std::move(expr)
    };
}


c4::ast2::expression
c4::p2::parser::parse_final_expression() {
    const auto lpar = expect_token<tokens::lparen>();
    if (lpar) {
        next_relevant(); // (
        const auto expr = parse_expression();
        next_relevant(); // ) expected

        if (const auto rpar = expect_token<tokens::rparen>();
            !rpar)
            report_failure(rpar);

        return expr;
    }

    const auto str = expect_token<tokens::string_literal>();
    if (str) {
        const auto str_lit = parse_string_literal();
        return {
            str_lit.position(),
            str_lit.file_source(),
            str_lit.length(),
            str_lit
        };
    }

    const auto integer = expect_token<tokens::integer_literal>();
    if (integer) {
        const auto int_lit = parse_integer_literal();
        return {
            int_lit.position(),
            int_lit.file_source(),
            int_lit.length(),
            int_lit
        };
    }

    const auto symbol = expect_token<tokens::symbol>();
    if (symbol) {
        const auto sym = parse_symbol();
        return {
            sym.position(),
            sym.file_source(),
            sym.length(),
            sym
        };
    }

    const auto prefix_op = expect_token<tokens::operator_>();
    if (prefix_op) {
        next_relevant();
        ensure_valid_prefix_operator(*prefix_op);
        const auto op_sym = ast2::op_symbol(prefix_op->token_position(),
                                            prefix_op->source_name(),
                                            prefix_op->value().size(),
                                            prefix_op->value(),
                                            1);
        const auto expr = parse_final_expression();
        return ast2::expression(
            ast2::unary_op_call(
                prefix_op->token_position(),
                prefix_op->source_name(),
                prefix_op->value().size() + expr.length(),
                op_sym,
                std::move(ast2::expression_ptr(new ast2::expression(expr)))));
    }

    const auto fn_symbol = expect_token<tokens::bare_symbol>();
    if (fn_symbol) {
        const auto sym = parse_symbol();
        const auto known_sym_it = std::ranges::find_if(_scope_symbols.rbegin(), _scope_symbols.rend(),
                                                       [&sym](const auto scope_sym) {
                                                           return scope_sym == sym;
                                                       });
        if (known_sym_it == _scope_symbols.rend()) {
            fmt::print("{}\n",
                       source_diagnostic::error(
                           fmt::format("unknown symbol referenced in function call: {}", sym.name()),
                           sym.file_source(),
                           sym.position(),
                           sym.length()
                       ));
            throw bad_token_error{};
        }

        std::vector<ast2::expression> args;
        const auto params = known_sym_it->arity;
        for (unsigned i = 0; i < params; ++i) {
            args.push_back(parse_expression());
            next_relevant();
        }

        return ast2::expression(
            ast2::fn_call(
                sym.position(),
                sym.file_source(),
                sym.length(),
                sym,
                args
            ));
    }

    report_failure(lpar, str, integer, symbol, prefix_op, fn_symbol);
}

c4::ast2::expression
c4::p2::parser::parse_operator_precedence(ast2::expression&& lhs, unsigned precedence) {
    auto lookahead = expect_token<tokens::operator_>();
    auto ret = std::move(lhs);
    if (lookahead) {
        auto op_token = *lookahead;
        auto op = ensure_valid_operator(op_token);
        while (op.precedence >= precedence) {
            next_relevant();
            auto rhs = parse_final_expression();
            lookahead = expect_token<tokens::operator_>();
            if (lookahead) {
                auto op_ahead = ensure_valid_operator(*lookahead);

                while (lookahead && (op_ahead.precedence > op.precedence
                                     || (op_ahead.right_assoc && op_ahead.precedence == op.precedence))) {
                    rhs = parse_operator_precedence(std::move(rhs),
                                                    op.precedence + (op_ahead.precedence > op.precedence));
                    lookahead = expect_token<tokens::operator_>();
                    if (lookahead) op_ahead = ensure_valid_operator(*lookahead);
                }
            }
            ret = ast2::expression(ast2::binary_op_call(
                    op_token.token_position(),
                    op_token.source_name(),
                    op_token.value().size(),
                    ast2::op_symbol(op_token.token_position(),
                                    op_token.source_name(),
                                    op_token.value().size(),
                                    op_token.value(),
                                    2),
                    ast2::expression_ptr(new ast2::expression(std::move(ret))),
                    ast2::expression_ptr(new ast2::expression(std::move(rhs)))
                )
            );

            if (lookahead) {
                op_token = *lookahead;
                op = ensure_valid_operator(*lookahead);
            }
            else {
                break;
            }
        }
    }

    return ret;
}

bool
c4::p2::parser::next_relevant() {
    do {
        _current = _lexer.next();
        if (!_current) return false;
    } while (std::visit(token_ignorer{_valid}, *_current));
    return true;
}

void
c4::p2::parser::enter_scope() {
    _scope_symbol_size.emplace_back();
    _scope_operator_size.emplace_back();
    _scope_prefix_operator_size.emplace_back();
}

void
c4::p2::parser::leave_scope() {
    const auto symbols = _scope_symbol_size.back();
    _scope_symbol_size.pop_back();
    for (unsigned i = 0U; i < symbols; ++i) {
        _scope_symbols.pop_back();
    }

    const auto operators = _scope_operator_size.back();
    _scope_operator_size.pop_back();
    for (unsigned i = 0U; i < operators; ++i) {
        _scope_operators.pop_back();
    }

    const auto pfx_operators = _scope_prefix_operator_size.back();
    _scope_prefix_operator_size.pop_back();
    for (unsigned i = 0U; i < pfx_operators; ++i) {
        _scope_prefix_operators.pop_back();
    }
}

c4::p2::parser::prefix_operator_symbol&
c4::p2::parser::ensure_valid_prefix_operator(const tokens::operator_& sym) {
    if (const auto op = find_prefix_operator(sym.value())) return *op;
    fmt::print("{}\n",
               source_diagnostic::error(
                   fmt::format("unknown prefix operator referenced: {}/1", sym.value()),
                   sym.source_name(),
                   sym.token_position(),
                   sym.value().size()
               ));

    if (sym.value().size() > 1) {
        auto pos = sym.token_position().snapshot();
        auto line = std::string(pos.line);
        line.insert(pos.col_number, 1, ' ');
        ++pos.col_number;
        pos.line = line;
        fmt::print("{}\n", source_diagnostic::suggestion(
                       "if you meant to apply multiple prefix operators in sequence, separate them with whitespace or parentheses",
                       sym.source_name(),
                       pos,
                       1
                   ));
    }
    if (find_operator(sym.value()))
        fmt::print("{}\n",
                   source_diagnostic::note(
                       fmt::format("there exists an infix operator with name {}/2, did you mean to call that?",
                                   sym.value()),
                       sym.source_name(),
                       sym.token_position(),
                       sym.value().size()
                   ));

    throw bad_token_error{};
}

c4::p2::parser::operator_symbol&
c4::p2::parser::ensure_valid_operator(const tokens::operator_& sym) {
    if (const auto op = find_operator(sym.value())) return *op;
    fmt::print("{}\n",
               source_diagnostic::error(
                   fmt::format("unknown infix operator referenced: {}/2", sym.value()),
                   sym.source_name(),
                   sym.token_position(),
                   sym.value().size()
               ));

    if (sym.value().size() > 1) {
        auto pos = sym.token_position().snapshot();
        auto line = std::string(pos.line);
        line.insert(pos.col_number, 1, ' ');
        ++pos.col_number;
        pos.line = line;
        fmt::print("{}\n", source_diagnostic::suggestion(
                       "if you meant to apply a prefix operator after an infix, separate them with whitespace or parentheses",
                       sym.source_name(),
                       pos,
                       1
                   ));
    }

    if (find_prefix_operator(sym.value()))
        fmt::print("{}\n",
                   source_diagnostic::note(
                       fmt::format("there exists a prefix operator with name {}/1, did you mean to call that?",
                                   sym.value()),
                       sym.source_name(),
                       sym.token_position(),
                       sym.value().size()
                   ));

    throw bad_token_error{};
}

namespace {
    template<class T>
    T*
    find_any_operator(std::span<T> operators, std::string_view name) {
        const auto it = std::find_if(operators.rbegin(), operators.rend(),
                                     [&name](const auto& op) {
                                         return op.name == name;
                                     });
        if (it == operators.rend()) return nullptr;
        return &*it;
    }
}

c4::p2::parser::operator_symbol*
c4::p2::parser::find_operator(const std::string_view name) {
    return find_any_operator(std::span(_scope_operators), name);
}

c4::p2::parser::prefix_operator_symbol*
c4::p2::parser::find_prefix_operator(const std::string_view name) {
    return find_any_operator(std::span(_scope_prefix_operators), name);
}

void
c4::p2::parser::declare_symbol(std::string_view symbol, unsigned arity) {
    ++_scope_symbol_size.back();
    _scope_symbols.emplace_back(symbol, arity);
}

void
c4::p2::parser::declare_binop(std::string_view symbol, unsigned precedence, bool right_assoc) {
    ++_scope_operator_size.back();
    _scope_operators.emplace_back(symbol, precedence, right_assoc);
}

void
c4::p2::parser::declare_uniop(std::string_view symbol) {
    ++_scope_prefix_operator_size.back();
    _scope_prefix_operators.emplace_back(symbol);
}
