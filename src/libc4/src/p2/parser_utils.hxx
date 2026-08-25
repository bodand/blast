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
 * Originally created: 2026-08-23.
 *
 * src/c4/src/p2/parser_utils --
 *   
 */
#ifndef C4_PARSER_UTILS_HXX
#define C4_PARSER_UTILS_HXX

#include <c4/p2/lex/tokens.hxx>

#include <c4/diagnostic.hxx>

namespace c4::p2 {
	struct token_ignorer final {
		diagnostics_engine& diag;

		bool
		operator()(const tokens::whitespace&) const { return true; }

		bool
		operator()(const tokens::comment&) const { return true; }

		bool
		operator()(const tokens::unknown& unk) const {
			// format position before passing it to source_diagnostic to escape
			// the likely unprintable characters:
			auto pos = unk.token_position();
			const auto debug_line = fmt::format("{:?}", pos.expanded_range);
			pos.expanded_range = debug_line;
			diag.error(pos, "unknown characters found: {:?}", unk.value())
				 .note("line is escaped because of possibly unprintable characters, column location might be incorrect");
			return true;
		}

		bool
		operator()(const auto&) const { return false; }
	};

	template<class E1, class... Es>
	[[noreturn]] void
	report_failure(diagnostics_engine& diag,
						E1&& error,
						Es&&... trailing) {
		const auto& e1_error = error.error();
		diag.error(e1_error.position(), "encountered unexpected {}", e1_error.name())
			 .when(sizeof...(trailing) > 0)
			 .note("expected one of: {}", fmt::join(std::make_tuple(trailing.error().expected_token_name()...), ", "));
		throw bad_token_error{};
	}

	inline bool
	is_eof(const tokens::token_type& tok) noexcept {
		return std::holds_alternative<tokens::eof>(tok);
	}

	inline std::string_view
	name_of(const tokens::token_type& token) {
		return std::visit([](const auto& tok) {
			return tok.token_name;
		}, token);
	}

	inline position
	position_of(const tokens::token_type& token) {
		return std::visit([](const auto& tok) {
			return tok.token_position();
		}, token);
	}

	struct namespaced_symbol_attribute : c4::ast2::tags::typed_attribute<std::vector<c4::ast2::symbol>> {
		explicit
		namespaced_symbol_attribute(std::vector<c4::ast2::symbol>&& symbols)
			: typed_attribute(symbols) { }
	};

	struct nested_symbol_attribute : c4::ast2::tags::typed_attribute<c4::ast2::block*> {
		explicit
		nested_symbol_attribute(c4::ast2::block* blk)
			: typed_attribute(blk) { }
	};

	struct native_attachment : c4::ast2::tags::typed_attribute<c4::ast2::symbol> {
		explicit
		native_attachment(c4::ast2::symbol&& sym)
			: typed_attribute(sym) { }
	};

	struct flag_attribute : ast2::tags::typed_attribute<bool> {
		explicit
		flag_attribute()
			: typed_attribute(true) { }
	};
}

#endif
