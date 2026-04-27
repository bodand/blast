/* blAST project
 *
 * Copyright (c) 2026 András Bodor <bodand@pm.me>
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
 * Originally created: 2026-03-03.
 *
 * src/c4/include/c4/ast3_mapper --
 *   
 */
#ifndef BLAST_AST3_MAPPER_HXX
#define BLAST_AST3_MAPPER_HXX

#include <stack>
#include <vector>
#include <algorithm>
#include <ranges>
#include <map>
#include <string_view>
#include <optional>
#include <c4/visitor/visitor.hxx>

#include <c4/ast3/ast_context.hxx>

namespace c4::ast2 {
struct block;
struct block_args;
struct let_expression;
struct expression;
struct fn_call;
struct binary_op_call;
struct unary_op_call;
struct float_literal;
struct integer_literal;
struct string_literal;
struct dynamic_call;
struct symbol;
}

namespace c4::ast3 { }

namespace c4 {
struct ast3_mapper final : ast2::visitor<
			ast2::block,
			ast2::dynamic_call,
			ast2::float_literal,
			ast2::fn_call,
			ast2::integer_literal,
			ast2::let_expression,
			ast2::binary_op_call,
			ast2::unary_op_call,
			ast2::string_literal,
			ast2::expression
		> {
	explicit ast3_mapper(ast3::ast_context& context)
		: _context{context} { }

	/* Literals */
	void do_visit(const ast2::float_literal& obj) override;

	void do_visit(const ast2::integer_literal& obj) override;

	void do_visit(const ast2::string_literal& obj) override;

	/* Function call-like */
	void do_visit(const ast2::fn_call& obj) override;

	void do_visit(const ast2::binary_op_call& obj) override;

	void do_visit(const ast2::unary_op_call& obj) override;

	void do_visit(const ast2::dynamic_call& obj) override;

	/* Structure */
	void do_visit(const ast2::block& obj) override;

	void do_visit(const ast2::let_expression& obj) override;

	void do_visit(const ast2::expression& obj) override;

private:
	void
	define_function(ast2::block& obj);

	struct function_stack_element {
		explicit(false) function_stack_element(const std::vector<function_stack_element>& parents, std::string name)
			: _name{std::move(name)} {
			_parents.reserve(parents.size());
			std::ranges::transform(parents, std::back_inserter(_parents), [](const auto& e) { return e.name(); });
		}

		[[nodiscard]] const std::string&
		name() const noexcept { return _name; }

		[[nodiscard]] bool
		taken() const noexcept { return _taken; }

		[[nodiscard]] bool
		take() noexcept {
			if (_taken) return false;
			return _taken = true;
		}

		[[nodiscard]] std::string
		next_anonymous();

	private:
		std::string _name;
		std::vector<std::string> _parents;
		size_t _anonymous_counter{0};
		bool _taken{false};
	};

	void
	enter_function_definition(const std::string& fn);

	void
	leave_function_definition();

	function_stack_element*
	last_function();


private:
	ast3::ast_context& _context;
	std::vector<function_stack_element> _function_stack;
};
}

#endif
