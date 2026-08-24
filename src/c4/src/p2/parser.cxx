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

#include <utility>

#include <c4/p2/parser.hxx>
#include <c4/p2/lex/tokens.hxx>

#include <fmt/format.h>
#include <libassert/assert.hpp>

#include "parser_utils.hxx"

c4::p2::parser::parser(ast2::ast_context& context, diagnostics_engine& diagnostics_engine, lexer&& lexer)
	: _diag{diagnostics_engine}
	, _context{context}
	, _lexer{std::move(lexer)}
	, _current{_lexer.next()} {
	while (std::visit(token_ignorer{_diag}, _current)) {
		_current = _lexer.next();
		if (is_eof(_current)) break;
	}
}

c4::ast2::integer_literal
c4::p2::parser::parse_integer_literal() {
	const auto literal = expect_token<tokens::integer_literal>();
	if (literal) {
		next_relevant();
		return ast2::integer_literal::from_token(*literal);
	}

	report_failure(_diag, literal);
}

c4::ast2::float_literal
c4::p2::parser::parse_float_literal() {
	const auto literal = expect_token<tokens::float_literal>();
	if (literal) {
		next_relevant();
		return ast2::float_literal::from_token(*literal);
	}

	report_failure(_diag, literal);
}

c4::ast2::string_literal
c4::p2::parser::parse_string_literal() {
	const auto literal = expect_token<tokens::string_literal>();
	if (literal) {
		next_relevant();
		return ast2::string_literal::from_token(*literal);
	}

	report_failure(_diag, literal);
}

c4::ast2::symbol
c4::p2::parser::parse_symbol(const bool advance) {
	const auto symbol = expect_token<tokens::symbol>();
	if (symbol) {
		if (advance) next_relevant();
		return ast2::symbol::from_token(*symbol);
	}

	const auto bare_symbol = expect_token<tokens::bare_symbol>();
	if (bare_symbol) {
		if (advance) next_relevant();
		return ast2::symbol::from_token(*bare_symbol);
	}

	report_failure(_diag, symbol, bare_symbol);
}

c4::ast2::symbol
c4::p2::parser::parse_op_symbol() {
	const auto op = expect_token<tokens::operator_symbol>();
	if (op) {
		next_relevant();
		return ast2::symbol::from_token(*op);
	}

	const auto bare_op = expect_token<tokens::fn_operator>();
	if (bare_op) {
		next_relevant();
		return ast2::symbol::from_token(*bare_op);
	}

	report_failure(_diag, op, bare_op);
}

c4::ast2::symbol
c4::p2::parser::parse_bare_symbol() {
	const auto bare_symbol = expect_token<tokens::bare_symbol>();
	if (!bare_symbol) report_failure(_diag, bare_symbol);

	next_relevant();
	return ast2::symbol::from_token(*bare_symbol);
}

c4::ast2::expression*
c4::p2::parser::parse_use_expression() {
	const auto maybe_use = expect_token<tokens::use>();
	if (!maybe_use) report_failure(_diag, maybe_use);
	next_relevant();

	return nullptr;
}

c4::ast2::expression*
c4::p2::parser::parse_expression() {
	if (const auto empty = expect_token<tokens::semicolon>()) {
		next_relevant();
		return nullptr;
	}

	if (const auto use = expect_token<tokens::use>())
		return parse_use_expression();

	if (const auto let = expect_token<tokens::let>())
		return parse_let_expression();

	const auto lhs = parse_final_expression();
	return parse_operator_precedence(lhs, 0);
}

c4::ast2::expression*
c4::p2::parser::
parse_operator_let(const enum ast2::let_expression::visibility vis,
                   bool native) {
	auto op = parse_op_symbol();
	const auto let = _context.build_let_expression(
		op.position(),
		op,
		nullptr,
		vis
	);

	if (native) {
		_diag.error(op.position(),
		            "operator `{}/{}' cannot be declared native",
		            op.name(), op.base_arity())
		     .note("native modifier is ignored")
		     .note("consider defining a private function and wraping it",
		           op.name());
	}

	if (const auto last = _st.find_scoped_symbol_with_arity(op)) {
		_diag.error(op.position(),
		            "operator `({})/{}' is already defined",
		            op.name(), op.base_arity())
		     .note("replacing definition with this one for further parsing")
		     .when(last->symbol.referee)
		     .note(last->symbol.referee->position(), "previous definition is here")
		     .when(!last->symbol.referee)
		     .note("previous definition was externally provided");
	}

	std::string_view diagnostic;
	std::string_view continuation_note;
	parser_symbol* sym = nullptr;

	switch (op.base_arity()) {
	case 0:
		_diag.error(op.position(),
		            "invalid operator arity: {} for operator `({})/0' (expected 1 or 2)",
		            op.base_arity(),
		            op.name())
		     .note("reparsing as if it had one argument (prefix)");
		op = op.with_arity(1);
		[[fallthrough]];
	case 1:
		diagnostic = "operator `{}' is defined with one parameter (prefix) but definition expects `{}' arguments";
		continuation_note = "continuing parsing as if `{}' had one parameter (prefix)";

		sym = &_st.declare(op.name(),
		                   op.base_arity(),
		                   nullptr,
		                   static_cast<unsigned>(-1));
		break;

	default:
		_diag.error(op.position(),
		            "invalid operator arity: {} for operator `({})/?' (expected 1 or 2)",
		            op.base_arity(),
		            op.name())
		     .note("reparsing as if it had two arguments (infix)");
		op = op.with_arity(2);
		[[fallthrough]];
	case 2:
		diagnostic = "operator `{}' is defined with two parameters (infix) but definition expects `{}' arguments";
		continuation_note = "continuing parsing as if `{}' had two parameters (infix)";

		const auto left_assoc = parse_associativity_indicator(op.name());
		next_relevant();
		const unsigned precedence = parse_precedence(op.name());
		next_relevant();
		op.set_op_data(left_assoc, precedence);

		sym = &_st.declare(op.name(),
		                   op.base_arity(),
		                   nullptr,
		                   precedence,
		                   !left_assoc);
		break;
	}

	const auto expr = parse_expression_of_let(op, let);

	if (expr) {
		if (unsigned unbound = expr->unbound_parameters();
			unbound != op.base_arity()) {
			_diag.error(op.position(),
			            fmt::runtime(diagnostic),
			            op.name(),
			            unbound)
			     .note(expr->position(), "definition is here")
			     .note(fmt::runtime(continuation_note), op.name());
		}
	}

	let->expression(expr);
	if (_within_block) let->emplace_attribute<nested_symbol_attribute>("nested-in", _within_block);
	sym->referee = let;
	return _context.build_expression(let);
}

c4::ast2::expression*
c4::p2::parser::parse_fn_let(enum ast2::let_expression::visibility vis,
                             const bool native) {
	const auto symbol = parse_symbol(false);

	ast2::let_expression* let = nullptr;

	if (const auto last = _st.find_scoped_symbol(symbol)) {
		if (last->symbol.referee) {
			let = dynamic_cast<ast2::let_expression*>(last->symbol.referee);
			if (let) {
				if (let->symbol().base_arity() != symbol.base_arity()) {
					_diag.error(symbol.position(),
					            "function `{}' is already defined with different arity",
					            symbol.name())
					     .note(last->symbol.referee->position(), "previous definition is here with arity {}",
					           let->symbol().base_arity())
					     .note("using previous definition for further parsing");
				}
				else if (!let->declaration()) {
					_diag.error(symbol.position(),
					            "function `{}' is already defined",
					            symbol.name())
					     .note("replacing definition with this one for further parsing")
					     .note(last->symbol.referee->position(), "previous definition is here");
				}
			}
			else {
				_diag.warning(symbol.position(),
				              "local variable shadows argument of enclosing block");
			}
		}
	}

	if (!let) {
		let = _context.build_let_expression(
			symbol.position(),
			symbol,
			nullptr,
			vis
		);
		_st.declare(symbol.name(), symbol.base_arity(), let);
	}

	if (native) {
		if (!expect_token<tokens::symbol>()) {
			_diag.warning(current_position(),
			              "native function `{}/{}' is declared with bare symbol (without arity)",
			              symbol.name(),
			              symbol.base_arity())
			     .note("consider using {}/0", symbol.name());
		}

		let->emplace_attribute<native_attachment>("native",
		                                          symbol.with_native());
		next_relevant();

		if (!expect_token<tokens::semicolon>()) {
			_diag.error(let->position(),
			            "function `{}/{}' is marked as `native' but given definition",
			            symbol.name(),
			            symbol.base_arity())
			     .note(current_position(),
			           "expected `;' to follow declaration");
		}
		next_relevant(); // skip semicolon

		if (_within_block) {
			let->emplace_attribute<nested_symbol_attribute>("nested-in", _within_block);
			_diag.error(let->position(),
			            "function `{}/{}' is marked as `native' but is nested",
			            symbol.name(),
			            symbol.base_arity())
			     .note(_within_block->position(), "enclosing block is here")
			     .when(_within_let)
			     .note(_within_let->position(), "nested within this let");
		}

		std::vector symbol_stack{symbol};
		let->emplace_attribute<namespaced_symbol_attribute>("symbol-stack", std::move(symbol_stack));

		return _context.build_expression(let);
	}
	next_relevant(); // advance manually because we did not ask parse_symbol to

	const auto expr = parse_expression_of_let(symbol, let);

	if (expr) {
		if (unsigned unbound = expr->unbound_parameters();
			unbound != symbol.base_arity()) {
			_diag.error(symbol.position(),
			            "function `{}' is defined with `{}' parameter(s) but definition expects `{}' arguments",
			            symbol.name(),
			            symbol.base_arity(),
			            unbound)
			     .note(expr->position().snapshot(), "definition is here")
			     .note("continuing parsing as if `{}' had `{}' parameter(s)",
			           symbol.name(),
			           symbol.base_arity());
		}
	}

	if (!let->value()) let->expression(expr);
	if (_within_block) let->emplace_attribute<nested_symbol_attribute>("nested-in", _within_block);
	return _context.build_expression(let);
}

c4::ast2::expression*
c4::p2::parser::parse_let_expression() {
	if (const auto let = expect_token<tokens::let>();
		!let)
		report_failure(_diag, let);
	next_relevant();

	auto vis = ast2::let_expression::v_internal;
	if (const auto vis_tok = expect_token<tokens::operator_>()) {
		if (vis_tok->value() == "+") vis = ast2::let_expression::v_public;
		else if (vis_tok->value() == "-") vis = ast2::let_expression::v_private;
		else if (vis_tok->value() == "~") vis = ast2::let_expression::v_internal;
		else report_failure(_diag, vis_tok);

		next_relevant();
	}

	bool native = false;
	if (const auto bare_symbol = expect_token<tokens::bare_symbol>()) {
		if (bare_symbol->name() == "native") {
			native = true;
			next_relevant();
		}
	}

	if (expect_token<tokens::operator_symbol>()
	    || expect_token<tokens::fn_operator>())
		return parse_operator_let(vis, native);

	return parse_fn_let(vis, native);
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
			report_failure(_diag, rpar);

		next_relevant();
		return expr;
	}

	const auto str = expect_token<tokens::string_literal>();
	if (str) return _context.build_expression(parse_string_literal());

	const auto flt = expect_token<tokens::float_literal>();
	if (flt) return _context.build_expression(parse_float_literal());

	const auto integer = expect_token<tokens::integer_literal>();
	if (integer) return _context.build_expression(parse_integer_literal());

	const auto symbol = expect_token<tokens::symbol>();
	if (symbol) return _context.build_expression(parse_symbol());

	const auto lbrace = expect_token<tokens::lbrace>();
	if (lbrace) return _context.build_expression(parse_block());

	const auto backslash = expect_token<tokens::backslash>();
	if (backslash) return _context.build_expression(parse_block());

	const auto prefix_op = expect_token<tokens::operator_>();
	if (prefix_op) {
		next_relevant();

		ensure_valid_prefix_operator(*prefix_op);
		const auto op_sym = ast2::symbol(prefix_op->token_position(),
		                                 prefix_op->value(),
		                                 1);
		const auto resolved = _st.find_scoped_symbol_with_arity(op_sym);
		DEBUG_ASSERT(resolved, "prefix operator should have been resolved (ensure_valid_prefix_operator called above)",
		             op_sym.name(),
		             op_sym.base_arity());
		op_sym.references(resolved->symbol.referee);

		std::vector<ast2::symbol> closure_symbols;
		if (resolved->save_in_context || _st.root()) {
			closure_symbols.push_back(op_sym);
			closure_symbols.back().references(resolved->symbol.referee);
		}

		const auto expr = parse_final_expression();
		const auto op = _context.build_unary_op_call(prefix_op->token_position(),
		                                             op_sym,
		                                             expr);
		return _context.build_expression(op, std::move(closure_symbols));
	}

	const auto fn_symbol = expect_token<tokens::bare_symbol>();
	if (fn_symbol) {
		auto sym = parse_symbol();
		const auto resolved = _st.find_scoped_symbol(sym);
		if (!resolved) {
			/// XXX add flaky recover logic, maybe we can produce something
			/// potentially helpful
			_diag.error(sym.position(),
			            "unknown symbol referenced in function call: {}",
			            sym.name());
			throw bad_token_error{};
		}
		sym = sym.with_arity(resolved->symbol.arity);
		sym.references(resolved->symbol.referee);

		std::vector<ast2::symbol> closure_symbols;
		if (resolved->save_in_context || _st.root()) {
			closure_symbols.push_back(sym);
			closure_symbols.back().references(resolved->symbol.referee);
		}

		std::vector<ast2::expression*> args;
		const auto params = resolved->symbol.arity;
		parse_n_expressions(params, args);

		const auto fn = _context.build_fn_call(sym.position(),
		                                       sym,
		                                       std::move(args));
		return _context.build_expression(fn, std::move(closure_symbols));
	}

	const auto dyn_call_start = expect_token<tokens::ampersand>();
	if (dyn_call_start) {
		next_relevant();

		auto expr = parse_expression();
		ASSERT(expr, "empty expression is unimplemented for dynamic calls");

		const auto dyn_call_end = expect_token<tokens::arity_marker>();
		if (!dyn_call_end) report_failure(_diag, dyn_call_end);
		next_relevant();

		std::vector<ast2::expression*> args;
		const auto params = dyn_call_end->arity();
		parse_n_expressions(params, args);

		// XXX make dynamic_call be positioned between (- dyn_call_{end,start})

		const auto dyn_call = _context.build_dynamic_call(dyn_call_start->token_position(),
		                                                  expr,
		                                                  std::move(args));
		return _context.build_expression(dyn_call);
	}

	report_failure(_diag, lpar, str, flt, integer, symbol, prefix_op, lbrace, backslash, fn_symbol, dyn_call_start);
}

void
c4::p2::parser::set_symbol_stack(const ast2::symbol& symbol,
                                 const ast2::let_expression* let,
                                 const ast2::let_expression* const memory) {
	if (memory) {
		if (const auto& stck = memory->attribute_value<std::vector<ast2::symbol>>("symbol-stack")) {
			std::vector symbol_stack(stck->begin(), stck->end());
			symbol_stack.push_back(symbol);
			let->emplace_attribute<namespaced_symbol_attribute>("symbol-stack", std::move(symbol_stack));
			return;
		}
	}

	std::vector symbol_stack{symbol};
	let->emplace_attribute<namespaced_symbol_attribute>("symbol-stack", std::move(symbol_stack));
}


c4::ast2::expression*
c4::p2::parser::parse_expression_of_let(const ast2::symbol& symbol,
                                        ast2::let_expression* let) {
	const auto memory = std::exchange(_within_let, let);
	_st.enter_scope();
	set_symbol_stack(symbol, let, memory);

	const auto expr = parse_expression();
	_st.leave_scope();
	std::exchange(_within_let, memory);

	return expr;
}

namespace {
	template<class... Args>
	c4::position
	position_of_either(c4::diagnostics_engine& diag, Args&&... args) {
		// XXX kinda hacky
		alignas(c4::position) std::byte data[sizeof(c4::position)];

		const bool found_pos = (
			(args && (new(data) c4::position(args->token_position()), true)) || ...
		);
		if (found_pos) return *reinterpret_cast<c4::position*>(data);

		c4::p2::report_failure(diag, std::forward<decltype(args)>(args)...);
	}
}

c4::ast2::block*
c4::p2::parser::parse_block() {
	std::vector<ast2::expression*> expressions;

	auto lbrace = expect_token<tokens::lbrace>();
	auto bslash = expect_token<tokens::backslash>();
	const auto pos = position_of_either(_diag, lbrace, bslash);
	next_relevant();

	auto block = _context.build_block(
		pos,
		std::vector<ast2::expression*>{},
		nullptr
	);

	const auto memory = std::exchange(_within_block, block);
	_st.enter_scope();
	ast2::block_args* args{};
	if (const auto args_pipe = expect_token<tokens::pipe>()) args = parse_block_args();

	if (lbrace) {
		auto next = expect_token<tokens::rbrace>();
		while (!next) {
			if (auto expr = parse_expression()) expressions.emplace_back(expr);
			next = expect_token<tokens::rbrace>();
		}
		next_relevant();
	}

	if (bslash) {
		if (auto expr = parse_expression()) expressions.emplace_back(expr);
	}

	block->args(args);
	block->expressions(std::move(expressions));
	_st.leave_scope();
	return std::exchange(_within_block, memory);
}

c4::ast2::block_args*
c4::p2::parser::parse_block_args() {
	const auto lead = expect_token<tokens::pipe>();
	if (!lead) report_failure(_diag, lead);
	next_relevant();

	std::vector<ast2::symbol> args{};
	auto sym = expect_token<tokens::bare_symbol>();
	while (sym) {
		args.emplace_back(parse_bare_symbol());
		sym = expect_token<tokens::bare_symbol>();
	}

	if (const auto tail = expect_token<tokens::pipe>();
		!tail)
		report_failure(_diag, tail);
	next_relevant();

	const auto block_args = _context.build_block_args(lead->token_position(),
	                                                  std::span(args));
	for (std::size_t i = 0; i < block_args->size(); ++i) {
		auto& argument = block_args->argument_reference(i);
		_st.declare(argument.name(), 0, &argument);
	}

	return block_args;
}

std::vector<c4::ast2::expression*>
c4::p2::parser::parse_script() {
	std::vector<ast2::expression*> expressions{};
	while (!is_eof(_current)) {
		auto expr = parse_expression();
		if (!expr) continue;

		if (!expr->attribute_value<bool>("emplaced")) {
			std::ignore = expr->emplace_attribute<flag_attribute>("emplaced");
			expressions.emplace_back(expr);
		}
	}
	return expressions;
}

bool
c4::p2::parser::parse_associativity_indicator(std::string_view op) {
	const auto bare_symbol = expect_token<tokens::bare_symbol>();
	if (!bare_symbol) {
		_diag.error(position_of(_current),
		            "expected associativity indicator (`left' or `right') found `{}'",
		            name_of(_current))
		     .note("continuing to parse as if `{}' was left associative", op);
		return true; // left-assoc
	}

	const auto assoc_direction = ast2::symbol::from_token(*bare_symbol);
	if (assoc_direction.name() == "right") return false;
	if (assoc_direction.name() == "left") return true;

	_diag.error(assoc_direction.position(),
	            "expected associativity indicator (`left' or `right') found `{}'",
	            assoc_direction.name())
	     .note("continuing to parse as if `{}' was left associative", op);
	return true;
}

unsigned
c4::p2::parser::parse_precedence(std::string_view op) {
	const auto int_lit = expect_token<tokens::integer_literal>();
	if (!int_lit) {
		_diag.error(position_of(_current),
		            "expected precedence value (0..{}) found `{}'",
		            cfg_max_precedence,
		            name_of(_current))
		     .note("continuing to parse as if `{}' had precedence of 0", op);
		return 0;
	}

	const auto uint = ast2::integer_literal::from_token(*int_lit);
	const auto uvalue = static_cast<unsigned>(uint.value());
	if (uvalue <= cfg_max_precedence) return uvalue;

	_diag.error(uint.position(),
	            "expected precedence value (0..{}) found `{}'",
	            cfg_max_precedence,
	            uint.value())
	     .note("continuing to parse as if `{}' had precedence of 0",
	           op);
	return 0;
}

void
c4::p2::parser::parse_n_expressions(const unsigned n,
                                    std::vector<ast2::expression*>& expressions) {
	for (unsigned i = 0; i < n; ++i) {
		auto expr = parse_expression();
		ASSERT(expr, "argument-list cutting via the empty expression is not implemented");
		expressions.emplace_back(expr);
	}
}

c4::ast2::expression*
c4::p2::parser::parse_operator_precedence(ast2::expression* lhs,
                                          const unsigned precedence) {
	auto lookahead = expect_token<tokens::operator_>();
	auto ret = lhs;
	if (lookahead) {
		auto op_token = *lookahead;
		auto op = ensure_valid_infix_operator(op_token);
		while (op.precedence >= precedence) {
			next_relevant();
			auto rhs = parse_final_expression();
			lookahead = expect_token<tokens::operator_>();
			if (lookahead) {
				auto op_ahead = ensure_valid_infix_operator(*lookahead);

				while (lookahead && (op_ahead.precedence > op.precedence
				                     || (op_ahead.right_assoc && op_ahead.precedence == op.precedence))) {
					rhs = parse_operator_precedence(rhs,
					                                op.precedence + (op_ahead.precedence > op.precedence));
					lookahead = expect_token<tokens::operator_>();
					if (lookahead) op_ahead = ensure_valid_infix_operator(*lookahead);
				}
			}

			ast2::symbol op_sym(op_token.token_position(),
			                    op_token.value(),
			                    2);

			const auto resolved = _st.find_scoped_symbol_with_arity(op_sym);
			DEBUG_ASSERT(resolved,
			             "prefix operator should have been resolved (ensure_valid_infix_operator called above)",
			             op_sym.name(),
			             op_sym.base_arity());
			op_sym.references(resolved->symbol.referee);

			std::vector<ast2::symbol> closure_symbols;
			if (resolved->save_in_context || _st.root()) {
				closure_symbols.push_back(op_sym);
				closure_symbols.back().references(resolved->symbol.referee);
			}

			const auto bin_op = _context.build_binary_op_call(op_token.token_position(),
			                                                  op_sym,
			                                                  ret,
			                                                  rhs);
			ret = _context.build_expression(bin_op, std::move(closure_symbols));

			if (!lookahead) break;
			op_token = *lookahead;
			op = ensure_valid_infix_operator(*lookahead);
		}
	}

	return ret;
}

void
c4::p2::parser::next_relevant() {
	do {
		_current = _lexer.next();
		if (is_eof(_current)) return;
	} while (std::visit(token_ignorer{_diag}, _current));
}


c4::p2::parser_symbol&
c4::p2::parser::ensure_valid_prefix_operator(const tokens::operator_& sym) {
	if (const auto op = _st.find_prefix_operator(sym.value())) return *op;

	auto pos = sym.token_position().snapshot();
	_diag.error(sym.token_position(),
	            "unknown prefix operator referenced: {}/1",
	            sym.value())
	     .when(_st.find_infix_operator(sym.value()))
	     .note("there exists an infix operator with name {}/2, did you mean to call that?", sym.value())
	     .when(sym.size() > 1)
	     .suggest([&pos] -> position&& {
		              auto& line = pos.attach(pos.expanded_range);
		              line.insert(pos.col_number + 2, 1, ' ');
		              pos.col_number_end = ++pos.col_number;
		              pos.expanded_range = line;
		              return std::move(pos);
	              }(),
	              "if you meant to apply multiple prefix operators in sequence,"
	              " separate them with whitespace or parentheses");

	throw bad_token_error{};
}

c4::p2::parser_symbol&
c4::p2::parser::ensure_valid_infix_operator(const tokens::operator_& sym) {
	if (const auto op = _st.find_infix_operator(sym.value())) return *op;

	auto pos = sym.token_position().snapshot();
	_diag.error(sym.token_position(),
	            "unknown infix operator referenced: {}/2",
	            sym.value())
	     .when(_st.find_prefix_operator(sym.value()))
	     .note("there exists an prefix operator with name {}/1, did you mean to call that?", sym.value())
	     .when(sym.size() > 1)
	     .suggest([&pos] -> position&& {
		              auto& line = pos.attach(pos.expanded_range);
		              line.insert(pos.col_number + 2, 1, ' ');
		              pos.col_number_end = ++pos.col_number;
		              pos.expanded_range = line;
		              return std::move(pos);
	              }(),
	              "if you meant to apply a prefix operator after an infix,"
	              " separate them with whitespace or parentheses");

	throw bad_token_error{};
}
