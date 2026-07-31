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

#include <algorithm>
#include <ranges>

#include <c4/ast_dumper.hxx>
#include <uni_algo/ranges.h>

void
c4::ast_dumper::do_visit(const ast2::block_args& obj) {
	_os << "[";
	auto args = obj.args();
	if (args.empty()) {
		_os << "]";
		return;
	}

	size_t i = 0;
	for (const auto& expr : args | std::views::take(args.size() - 1)) {
		expr->accept(*this);
		_os << "\033[2m#" << &obj.argument_reference(i++) << ", \033[m";
	}
	args.back()->accept(*this);
	_os << "\033[2m#" << &obj.argument_reference(i++) << "\033[m]";
}

void
c4::ast_dumper::do_visit(const ast2::block& obj) {
	_os << "{\033[34mlambda\033[m ";
	if (obj.args()) {
		obj.args()->accept(*this);
		_os << " ";
	}
	auto expressions = obj.expressions();
	if (expressions.empty()) {
		_os << "}";
		return;
	}

	_os << "\n" << std::string(2U * ++_depth, ' ');
	for (const auto& expr : expressions | std::views::take(expressions.size() - 1)) {
		expr->accept(*this);
		_os << "\n" << std::string(2U * _depth, ' ');
	}
	expressions.back()->accept(*this);
	_os << "}";
	--_depth;
}

void
c4::ast_dumper::do_visit(const ast2::dynamic_call& obj) {
	_os << "\033[1;3m(\033[m";
	obj.callee()->accept(*this);


	auto expressions = obj.args();
	if (expressions.empty()) {
		_os << "\033[1;3m)\033[m";
		return;
	}

	_os << "\n" << std::string(2U * _depth, ' ');
	for (const auto& expr : expressions | std::views::take(expressions.size() - 1)) {
		expr->accept(*this);
		_os << "\n" << std::string(2U * _depth, ' ');
	}
	expressions.back()->accept(*this);
	_os << "\033[1;3m)\033[m";
}

void
c4::ast_dumper::do_visit(const ast2::expression& obj) {
	_os << "{";
	if (obj.true_closure()) {
		_os << "closure ";
	}
	else {
		_os << "\033[2mexpr\033[m ";
	}

	if (const auto let = obj.owner()) {
		_os << "\033[2m@" << let << "\033[m ";
	}
	if (!obj.closure()) {
		_os << "\n" << std::string(2U * ++_depth, ' ');
		obj.accept_skip_self(*this);
		_os << "}";
		--_depth;
		return;
	}
	_os << "[";
	auto syms = obj.closure_symbols();
	for (const auto& sym : syms | std::views::take(syms.size() - 1)) {
		sym.accept(*this);
		_os << ", ";
	}
	syms.back().accept(*this);
	_os << "]\n" << std::string(2U * ++_depth, ' ');

	obj.accept_skip_self(*this);
	--_depth;
	_os << "}";
}

void
c4::ast_dumper::do_visit(const ast2::fn_call& obj) {
	_os << "\033[1m(";
	obj.sym().accept(*this);
	_os << "\033[m";

	auto expressions = obj.args();
	if (expressions.empty()) {
		_os << "\033[1m)\033[m";
		return;
	}

	_os << "\n" << std::string(2U * ++_depth, ' ');

	for (const auto& expr : expressions | std::views::take(expressions.size() - 1)) {
		expr->accept(*this);
		_os << "\n" << std::string(2U * _depth, ' ');
	}
	expressions.back()->accept(*this);
	_os << "\033[1m)\033[m";
	--_depth;
}

void
c4::ast_dumper::do_visit(const ast2::let_expression& obj) {
	_os << "{\033[34mlet\033[m";
	if (obj.introduces_variable()) {
		if (obj.value_constant()) {
			_os << "\033[2;34m(const)\033[m ";
		}
		else {
			_os << "\033[2;34m(var)\033[m ";
		}
	}
	else {
		if (obj.attribute_value<ast2::symbol>("native")) {
			_os << "\033[2;34m(native)\033[m ";
		}
		else {
			_os << "\033[2;34m(fn)\033[m ";
		}
	}

	_os << "\033[3m";
	obj.symbol().accept(*this);
	_os << " \033[2m@" << &obj << "\033[m";
	if (const auto& stck = obj.attribute_value<std::vector<ast2::symbol>>("symbol-stack")) {
		_os << " [\033[2;32m";
		std::ranges::copy(*stck | std::views::transform([](const auto& sym) { return sym.name(); }),
		                  std::ostream_iterator<std::string_view>(_os, "\033[39m, \033[32m"));
		// ", " always present because stack always contains at least current name
		_os << "\b\b\033[0m]";
	}
	_os << " \n" << std::string(2U * ++_depth, ' ');
	if (const auto val = obj.value()) {
		val->accept(*this);
	} else {
		_os << ";";
	}
	--_depth;
	_os << "}";
}

void
c4::ast_dumper::do_visit(const ast2::binary_op_call& obj) {
	_os << "\033[1m(";
	obj.op().accept(*this);
	_os << "\033[m";

	_os << "\n" << std::string(2U * ++_depth, ' ');
	obj.left().accept(*this);
	_os << "\n" << std::string(2U * _depth, ' ');
	obj.right().accept(*this);

	_os << "\033[1m)\033[m";
	--_depth;
}

void
c4::ast_dumper::do_visit(const ast2::unary_op_call& obj) {
	_os << "\033[1m(";
	obj.op().accept(*this);
	_os << "\033[m";

	_os << "\n" << std::string(2U * ++_depth, ' ');
	obj.operand().accept(*this);

	_os << "\033[1m)\033[m";
	--_depth;
}

void
c4::ast_dumper::do_visit(const ast2::symbol& obj) {
	_os << "\033[32m" << obj.name() << "\033[33m/" << obj.base_arity() << "\033[m";
	if (!obj.extern_()) {
		_os << "\033[2m->" << obj.references() << "\033[m";
	}
}
