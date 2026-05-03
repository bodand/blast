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
 * src/c4/include/c4/ast2/symbol --
 *   Either a bare-symbol (sym) or an elaborated symbol (sym/1) are stored here.
 *   The latter can stand by itself in a lot of spaces, but the former is mostly
 *   present as elements of let-expressions or fn-calls.
 */
#ifndef C4_AST2_SYMBOL_HXX
#define C4_AST2_SYMBOL_HXX

#include <string>
#include <string_view>

#include <c4/tags/visitable.hxx>
#include <c4/tags/source_positioned.hxx>
#include <c4/tags/evaluation_constness.hxx>
#include <c4/tags/referable.hxx>

namespace c4::ast2 {
	/**
    * Symbol for undefined but declared symbols. These are not inherently
    * present in the source.
    */
	struct undef_symbol {
		undef_symbol(const std::string_view& name, const unsigned arity)
			: _name{name}
			, _arity{arity} { }

		[[nodiscard]] std::string
		mangle() const;

		[[nodiscard]] std::string_view
		name() const {
			return _name;
		}

		[[nodiscard]] unsigned
		arity() const {
			return _arity;
		}

	private:
		std::string_view _name;
		unsigned _arity;
	};

	struct symbol final : tags::visitable
	                      , tags::dynamic_node
	                      , tags::source_positioned {
		template<class Token>
		[[nodiscard]] static symbol
		from_token(const Token& tok) {
			return {
				tok.token_position(),
				tok.name(),
				tok.arity()
			};
		}

		symbol(const c4::position& position,
		       std::string_view name,
		       unsigned arity);

		[[nodiscard]] std::string_view
		name() const noexcept { return _name; }

		[[nodiscard]] unsigned
		base_arity() const noexcept { return _arity; }

		[[nodiscard]] unsigned
		effective_arity() const {
			if (_references) return _references->effective_arity();
			return _arity;
		}

		[[nodiscard]] symbol
		with_arity(unsigned arity) const;

		[[nodiscard]] std::string
		mangle() const;

		friend bool
		operator<(const symbol& lhs, const symbol& rhs) {
			const auto cmp = lhs._name <=> rhs._name;
			if (std::is_lt(cmp)) return true;
			if (std::is_gt(cmp)) return false;
			return lhs._arity < rhs._arity;
		}

		friend bool
		operator<=(const symbol& lhs, const symbol& rhs) { return !(rhs < lhs); }

		friend bool
		operator>(const symbol& lhs, const symbol& rhs) { return rhs < lhs; }

		friend bool
		operator>=(const symbol& lhs, const symbol& rhs) { return !(lhs < rhs); }

		friend bool
		operator==(const symbol& lhs, const symbol& rhs) {
			return lhs._arity == rhs._arity
			       && lhs._name == rhs._name;
		}

		friend bool
		operator!=(const symbol& lhs, const symbol& rhs) { return !(lhs == rhs); }

		[[nodiscard]] tags::referable*
		references() const noexcept { return _references; }

		void
		references(tags::referable* ref) const noexcept;

		[[nodiscard]] unsigned
		unbound_parameters() const noexcept { return 0; }

	private:
		mutable tags::referable* _references{};
		std::string_view _name;
		unsigned _arity;
	};
}

#endif
