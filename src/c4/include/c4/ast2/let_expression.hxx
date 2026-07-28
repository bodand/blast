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
 * src/c4/include/c4/ast2/let_expression --
 *   
 */
#ifndef C4_AST2_LET_EXPRESSION_HXX
#define C4_AST2_LET_EXPRESSION_HXX

#ifndef __clang__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wchanges-meaning"
#endif

#include <c4/ast2/ast_node.hxx>
#include <c4/ast2/symbol.hxx>

#include <c4/tags/referable.hxx>
#include <c4/tags/source_positioned.hxx>
#include <c4/tags/visitable.hxx>

#include <string_view>

namespace c4::ast2 {
	struct expression;
	struct block;

	struct let_expression final : tags::referable
	                              , ast_node
	                              , tags::visitable {
		let_expression(const c4::position& position,
		               symbol symbol,
		               expression* expr);

		let_expression(const let_expression& cp) = delete;

		let_expression&
		operator=(const let_expression& cp) = delete;

		void
		expression(expression* expr);

		[[nodiscard]] bool
		value_constant() const noexcept override;

		let_expression(let_expression&&) noexcept = delete;

		let_expression&
		operator=(let_expression&&) noexcept = delete;

		[[nodiscard]] symbol
		symbol() const { return _symbol; }

		[[nodiscard]] const struct expression&
		value() const;

		[[nodiscard]] struct expression&
		value();

		std::string_view
		name() const override { return _symbol.name(); }

		[[nodiscard]] unsigned
		unbound_parameters() const noexcept { return 0; }

		[[nodiscard]] bool
		introduces_variable() const noexcept override;

		[[nodiscard]] bool
		introduces_function() const noexcept override { return !introduces_variable(); }

		[[nodiscard]] bool
		global_symbol() const;

		[[nodiscard]] block*
		function_body() const noexcept;

		[[nodiscard]] std::optional<unsigned>
		invocable_with() const noexcept { return std::nullopt; }

		[[nodiscard]] bool
		constant_evaluated(std::span<const struct symbol> skips) const noexcept override;

	private:
		void
		mark_expression_owned();

		struct symbol _symbol;
		struct expression* _value;
	};
}

#ifndef __clang__
#pragma GCC diagnostic pop
#endif

#endif
