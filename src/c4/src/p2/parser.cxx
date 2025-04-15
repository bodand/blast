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

#include <deque>
#include <iostream>
#include <ranges>

#include <c4/ast2/visitor/visitor.hxx>
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
c4::p2::parser::parse_op_symbol() {
    const auto op = expect_token<tokens::operator_symbol>();
    if (op) {
        next_relevant();
        return {
            op->token_position(),
            op->source_name(),
            op->value().size(),
            op->name(),
            op->arity()
        };
    }

    const auto bare_op = expect_token<tokens::fn_operator>();
    if (bare_op) {
        next_relevant();
        return {
            bare_op->token_position(),
            bare_op->source_name(),
            bare_op->value().size(),
            bare_op->name(),
            1
        };
    }

    report_failure(op, bare_op);
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

c4::ast2::expression*
c4::p2::parser::parse_expression() {
    if (const auto let = expect_token<tokens::let>()) {
        return parse_let_expression();
    }

    auto lhs = parse_final_expression();
    return parse_operator_precedence(lhs, 0);
}

namespace {
    struct referable_visitor final : c4::ast2::visitor<c4::ast2::expression, c4::ast2::let_expression> {
        c4::ast2::tags::referable*& out;

        explicit
        referable_visitor(c4::ast2::tags::referable*& out)
            : out{out} { }

        void
        do_visit(const c4::ast2::expression& obj) override {
            obj.accept_skip_self(*this);
        }

        void
        do_visit(const c4::ast2::let_expression& obj) override {
            out = const_cast<c4::ast2::let_expression*>(&obj);
        }
    };
}

c4::ast2::expression*
c4::p2::parser::parse_let_expression() {
    if (const auto let = expect_token<tokens::let>();
        !let)
        report_failure(let);
    next_relevant();

    // Symbol declaration happens immediately after parsing the symbol: this is
    // required to allow recursion. If symbol was declared at the end of the
    // let expression, the expression parsing after this could not refer to it
    // this is true for normal symbols as well as operator symbols

    if (expect_token<tokens::operator_symbol>()
        || expect_token<tokens::fn_operator>()) {
        if (const auto op = parse_op_symbol();
            op.arity() == 1) {
            declare_uniop(op.name());

            auto expr = parse_expression();
            const auto let = _context.build_let_expression(
                op.position(),
                op.file_source(),
                op.length(),
                ast2::symbol(op.position(), op.file_source(), op.length(), op.name(), op.arity()),
                expr
            );
            return _context.build_expression(let);
        }
        else {
            if (op.arity() == 2) {
                auto left_assoc = true;
                if (const auto assoc_direction = parse_bare_symbol();
                    assoc_direction.name() == "right") {
                    left_assoc = false;
                }
                else if (assoc_direction.name() != "left") {
                    _valid = false;
                    fmt::print("{}\n", source_diagnostic::error(fmt::format(
                                                                    "expected associativity indicator (`left' or `right') found `{}'",
                                                                    assoc_direction.name()),
                                                                assoc_direction.file_source(),
                                                                assoc_direction.position(),
                                                                assoc_direction.length()));
                }

                unsigned precedence = 0;
                if (const auto uint = parse_integer_literal();
                    uint.value() >= 0 && uint.value() < 10) {
                    precedence = uint.value();
                }
                else {
                    _valid = false;
                    fmt::print("{}\n", source_diagnostic::error(fmt::format(
                                                                    "expected precedence (0..10) found `{}'",
                                                                    uint.value()),
                                                                uint.file_source(),
                                                                uint.position(),
                                                                uint.length()));
                }
                declare_binop(op.name(), precedence, !left_assoc);

                auto expr = parse_expression();
                const auto let = _context.build_let_expression(
                    op.position(),
                    op.file_source(),
                    op.length(),
                    ast2::symbol(op.position(), op.file_source(), op.length(), op.name(), op.arity()),
                    expr
                );
                return _context.build_expression(let);
            }
            UNREACHABLE("operator's arity can only be 1 or 2", op);
        }
    }

    const auto symbol = parse_symbol();
    auto& sym = declare_symbol_internal(symbol.name(), symbol.arity(), nullptr);

    auto expr = parse_expression();
    const auto let = _context.build_let_expression(
        symbol.position(),
        symbol.file_source(),
        symbol.length(),
        symbol,
        expr
    );
    sym.referee = let;
    return _context.build_expression(let);
}


c4::ast2::expression*
c4::p2::parser::parse_final_expression() {
    const auto lpar = expect_token<tokens::lparen>();
    if (lpar) {
        next_relevant(); // (
        auto expr = parse_expression();
        // )
        if (const auto rpar = expect_token<tokens::rparen>();
            !rpar)
            report_failure(rpar);

        next_relevant();
        return expr;
    }

    const auto str = expect_token<tokens::string_literal>();
    if (str) {
        const auto str_lit = parse_string_literal();
        return _context.build_expression(str_lit);
    }

    const auto integer = expect_token<tokens::integer_literal>();
    if (integer) {
        const auto int_lit = parse_integer_literal();
        return _context.build_expression(int_lit);
    }

    const auto symbol = expect_token<tokens::symbol>();
    if (symbol) {
        const auto sym = parse_symbol();
        return _context.build_expression(sym);
    }

    const auto lbrace = expect_token<tokens::lbrace>();
    const auto backslash = expect_token<tokens::backslash>();
    if (lbrace || backslash) {
        const auto block = parse_block();
        return _context.build_expression(block);
    }

    const auto prefix_op = expect_token<tokens::operator_>();
    if (prefix_op) {
        next_relevant();
        ensure_valid_prefix_operator(*prefix_op);
        const auto op_sym = ast2::symbol(prefix_op->token_position(),
                                         prefix_op->source_name(),
                                         prefix_op->value().size(),
                                         prefix_op->value(),
                                         1);
        const auto expr = parse_final_expression();
        const auto op = _context.build_unary_op_call(prefix_op->token_position(),
                                                     prefix_op->source_name(),
                                                     prefix_op->value().size() + expr->length(),
                                                     op_sym,
                                                     expr);
        return _context.build_expression(op);
    }

    const auto fn_symbol = expect_token<tokens::bare_symbol>();
    if (fn_symbol) {
        auto sym = parse_symbol();
        const auto resolved = find_scoped_symbol(sym);
        if (!resolved) {
            fmt::print("{}\n",
                       source_diagnostic::error(
                           fmt::format("unknown symbol referenced in function call: {}", sym.name()),
                           sym.file_source(),
                           sym.position(),
                           sym.length()
                       ));
            throw bad_token_error{};
        }
        sym = sym.with_arity(resolved->symbol.arity);
        sym.references(resolved->symbol.referee);

        std::vector<ast2::symbol> closure_symbols;
        if (resolved->from_parent_scope) {
            closure_symbols.push_back(sym);
            closure_symbols.back().references(resolved->symbol.referee);
        }

        std::vector<ast2::expression*> args;
        const auto params = resolved->symbol.arity;
        parse_n_expressions(params, args);

        const auto fn = _context.build_fn_call(sym.position(),
                                               sym.file_source(),
                                               sym.length(),
                                               sym,
                                               std::move(args));
        return _context.build_expression(fn, std::move(closure_symbols));
    }

    const auto dyn_call_start = expect_token<tokens::ampersand>();
    if (dyn_call_start) {
        next_relevant();

        auto expr = parse_expression();

        const auto dyn_call_end = expect_token<tokens::arity_marker>();
        if (!dyn_call_end) report_failure(dyn_call_end);
        next_relevant();

        std::vector<ast2::expression*> args;
        const auto params = dyn_call_end->arity();
        parse_n_expressions(params, args);

        const auto dyn_call = _context.build_dynamic_call(dyn_call_start->token_position(),
                                                          dyn_call_start->source_name(),
                                                          static_cast<std::size_t>(
                                                              dyn_call_end->begin() - dyn_call_start->begin()),
                                                          expr,
                                                          std::move(args));
        return _context.build_expression(dyn_call);
    }

    report_failure(lpar, str, integer, symbol, prefix_op, lbrace, backslash, fn_symbol, dyn_call_start);
}

c4::ast2::block*
c4::p2::parser::parse_block() {
    const auto lbrace = expect_token<tokens::lbrace>();
    if (lbrace) {
        next_relevant();
        enter_scope();

        ast2::block_args* args{};
        if (const auto args_pipe = expect_token<tokens::pipe>()) args = parse_block_args();

        std::vector<ast2::expression*> expressions;
        auto next = expect_token<tokens::rbrace>();
        while (!next) {
            expressions.emplace_back(parse_expression());
            next = expect_token<tokens::rbrace>();
        }
        next_relevant();

        leave_scope();
        return _context.build_block(
            lbrace->token_position(),
            lbrace->source_name(),
            static_cast<std::size_t>(next->begin() - lbrace->begin()),
            std::move(expressions),
            args

        );
    }

    const auto bslash = expect_token<tokens::backslash>();
    if (bslash) {
        next_relevant();
        enter_scope();

        ast2::block_args* args{};
        if (const auto args_pipe = expect_token<tokens::pipe>()) args = parse_block_args();

        std::vector<ast2::expression*> expr;
        expr.emplace_back(parse_expression());

        const auto next = std::visit([](const auto& x) { return x.begin(); }, *_current);

        leave_scope();
        return _context.build_block(
            lbrace->token_position(),
            lbrace->source_name(),
            static_cast<std::size_t>(next - lbrace->begin()),
            std::move(expr),
            args
        );
    }

    report_failure(lbrace, bslash);
}


std::vector<c4::ast2::undef_symbol>
c4::p2::parser::promised_symbols() const {
    std::vector<ast2::undef_symbol> undef_symbols;
    for (const auto& scope_symbol : _scope_symbols) undef_symbols.emplace_back(scope_symbol.name, scope_symbol.arity);
    for (const auto& scope_symbol : _scope_operators) undef_symbols.emplace_back(scope_symbol.name, 2);
    for (const auto& [name] : _scope_prefix_operators) undef_symbols.emplace_back(name, 1);
    return undef_symbols;
}

void
c4::p2::parser::parse_n_expressions(const unsigned n,
                                    std::vector<ast2::expression*>& expressions) {
    for (unsigned i = 0; i < n; ++i) {
        expressions.emplace_back(parse_expression());
    }
}

c4::ast2::expression*
c4::p2::parser::parse_operator_precedence(ast2::expression* lhs, unsigned precedence) {
    auto lookahead = expect_token<tokens::operator_>();
    auto ret = lhs;
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
            ast2::symbol op_sym(op_token.token_position(),
                                op_token.source_name(),
                                op_token.value().size(),
                                op_token.value(),
                                2);
            const auto bin_op = _context.build_binary_op_call(op_token.token_position(),
                                                              op_token.source_name(),
                                                              op_token.value().size(),
                                                              op_sym,
                                                              ret,
                                                              rhs);
            ret = _context.build_expression(bin_op);

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
    template<class It>
    auto
    find_any_operator(It begin, It end, std::string_view name) -> typename std::iterator_traits<It>::value_type* {
        const auto it = std::find_if(begin, end,
                                     [&name](const auto& op) {
                                         return op.name == name;
                                     });
        if (it == end) return nullptr;
        return &*it;
    }
}

c4::p2::parser::operator_symbol*
c4::p2::parser::find_operator(const std::string_view name) {
    return find_any_operator(_scope_operators.rbegin(), _scope_operators.rend(), name);
}

c4::p2::parser::prefix_operator_symbol*
c4::p2::parser::find_prefix_operator(const std::string_view name) {
    return find_any_operator(_scope_prefix_operators.rbegin(), _scope_prefix_operators.rend(), name);
}

c4::ast2::block_args*
c4::p2::parser::parse_block_args() {
    const auto lead = expect_token<tokens::pipe>();
    if (!lead) report_failure(lead);
    next_relevant();

    std::vector<ast2::symbol> args{};
    auto sym = expect_token<tokens::bare_symbol>();
    while (sym) {
        args.emplace_back(parse_bare_symbol());
        sym = expect_token<tokens::bare_symbol>();
    }

    const auto tail = expect_token<tokens::pipe>();
    if (!tail) report_failure(tail);
    next_relevant();

    const auto block_args = _context.build_block_args(lead->token_position(),
                                                      lead->source_name(),
                                                      static_cast<std::size_t>(tail->begin() - lead->begin()),
                                                      std::span(args));
    for (std::size_t i = 0; i < block_args->size(); ++i) {
        auto& argument = block_args->argument_reference(i);
        declare_symbol_internal(argument.name(), 0, &argument);
    }

    return block_args;
}

std::vector<c4::ast2::expression*>
c4::p2::parser::parse_script() {
    std::vector<ast2::expression*> expressions{};
    for (;;) {
        if (!_current) return expressions;
        expressions.emplace_back(parse_expression());
    }
}

void
c4::p2::parser::declare_symbol(std::string_view symbol,
                               unsigned arity,
                               ast2::tags::referable* referee) {
    declare_symbol_internal(symbol, arity, referee);
}

c4::p2::parser::parser_symbol&
c4::p2::parser::declare_symbol_internal(std::string_view symbol,
                                        unsigned arity,
                                        ast2::tags::referable* referee) {
    ++_scope_symbol_size.back();
    return _scope_symbols.emplace_back(symbol, arity, referee);
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
