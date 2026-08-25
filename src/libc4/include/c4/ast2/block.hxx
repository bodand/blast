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
 * src/c4/include/c4/ast2/block --
 *   
 */
#ifndef C4_AST2_BLOCK_HXX
#define C4_AST2_BLOCK_HXX
#include <c4/tags/tailable.hxx>

#ifndef __clang__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wchanges-meaning"
#endif

#include <c4/tags/attributable.hxx>
#include <c4/tags/referable.hxx>
#include <c4/tags/source_positioned.hxx>
#include <c4/tags/visitable.hxx>

#include <c4/ast2/ast_node.hxx>
#include <c4/ast2/symbol.hxx>

#include <algorithm>
#include <string_view>
#include <iterator>
#include <span>
#include <utility>
#include <vector>

namespace c4::ast2 {
	struct expression;

	struct block_argument final : tags::referable
	                              , ast_node {
		explicit
		block_argument(symbol symbol);

		block_argument(block_argument&) = delete;

		block_argument&
		operator=(block_argument&) = delete;

		block_argument&
		operator=(block_argument&&) noexcept = delete;

		block_argument(block_argument&&) noexcept = delete;

		[[nodiscard]] const symbol&
		symbol() const noexcept { return _symbol; }

		std::string_view
		name() const override { return _symbol.name(); }

		[[nodiscard]] bool
		thunk() const noexcept override { return true; }

		[[nodiscard]] bool
		introduces_variable() const noexcept override { return true; }

		[[nodiscard]] bool
		introduces_function() const noexcept override { return false; }

		[[nodiscard]] bool
		declaration() const noexcept override { return false; }

	private:
		struct symbol _symbol;
	};

	struct block_args final : ast_node
	                          , tags::visitable
	                          , tags::source_positioned {
		block_args(const c4::position& position,
		           std::span<symbol> args);

		block_args(block_args&) = delete;

		block_args&
		operator=(block_args&) = delete;

		block_args(block_args&&) noexcept = delete;

		block_args&
		operator=(block_args&&) noexcept = delete;

		[[nodiscard]] std::vector<const symbol*>
		args() const {
			std::vector<const symbol*> result;
			result.reserve(_args.size());
			std::ranges::transform(
				_args, std::back_inserter(result),
				[](const auto& arg) { return &arg.symbol(); }
			);
			return result;
		}

		[[nodiscard]] std::span<const block_argument>
		block_arguments() const noexcept { return _args; }

		[[nodiscard]] std::size_t
		size() const noexcept { return _args.size(); }

		[[nodiscard]] block_argument&
		argument_reference(std::size_t arg_idx);

		[[nodiscard]] const block_argument&
		argument_reference(std::size_t arg_idx) const;

	private:
		std::vector<block_argument> _args{};
	};

	struct block final : ast_node
	                     , tags::visitable
	                     , tags::source_positioned
	                     , tags::attributable
	                     , tags::tailable {
		block(const c4::position& position,
		      std::vector<expression*>&& expressions,
		      block_args* args = nullptr);

		block(block& cp) = delete;

		block& operator=(const block&) = delete;

		block(block&&) noexcept = delete;

		block& operator=(block&&) noexcept = delete;

		[[nodiscard]] const block_args*
		args() const { return _args; }

		void
		args(block_args* args) { _args = args; }

		[[nodiscard]] unsigned
		arity() const noexcept {
			if (!_args) return 0U;
			return static_cast<unsigned>(_args->size());
		}

		/**
       * Generates a set of symbols that are used by the contained expressions
       * but are not resolved by the block's arguments or symbols defined
       * within the block.
       */
		[[nodiscard]] std::vector<symbol>
		effective_context_symbols() const;

		[[nodiscard]] std::span<const expression* const>
		expressions() const;

		void
		expressions(std::vector<expression*>&& expressions);

		[[nodiscard]] unsigned
		unbound_parameters() const noexcept {
			if (!_args) return 0U;
			return static_cast<unsigned>(_args->size());
		}

		[[nodiscard]] std::optional<unsigned>
		invocable_with() const noexcept { return unbound_parameters(); }

		[[nodiscard]] bool
		constant_evaluated(std::span<const symbol> skips) const noexcept override;

	private:
		block_args* _args{};
		std::vector<expression*> _expressions;
	};
}

#ifndef __clang__
#pragma GCC diagnostic pop
#endif

#endif
