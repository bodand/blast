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
 * Originally created: 2026-08-25.
 *
 * src/c4/src/p2/archive_parser.hxx/parse --
 *   
 */

#include <bit>

#include <c4/diagnostic.hxx>
#include <c4/p2/archive_parser.hxx>

#include <c4/ast2/ast_context.hxx>
#include <c4/ast2/symbol.hxx>

#include <LIEF/LIEF.hpp>

#include <libassert/assert.hpp>

static_assert(std::endian::native == std::endian::little
              || std::endian::native == std::endian::big,
              "Mixed endian hardware detected: please submit a patch");

namespace {
	using namespace c4::p2::a;

	bool
	is_eof(const tokens::token_type& tok) {
		return std::holds_alternative<tokens::eof>(tok);
	}

	struct parser_visitor final {
		void
		operator()(const tokens::error& err) {
			// only one error is supported before swallowing
			if (!error_counter++) return;
			_diag.error(c4::position::invalid_file_position(_src),
			            "error parsing archive: {}",
			            err.message);
		}

		void
		operator()(const tokens::eof&) {
			error_counter = 0;
		}

		void
		operator()(const tokens::metadata&) {
			error_counter = 0;
		}

		void
		operator()(const tokens::object& binary) {
			error_counter = 0;

			const auto obj = LIEF::Parser::parse(std::make_unique<LIEF::SpanStream>(binary.bytes));
			if (!obj) {
				_diag.error(c4::position::invalid_file_position(_src),
				            "error parsing object file: {}: god knows what's wrong with it though because LIEF",
				            binary.name);
				return;
			}

			const bool is_macho = obj->format() == LIEF::Binary::FORMATS::MACHO;
			const std::string_view xport_name = is_macho ? "__DWARF,__c4xport" : ".c4xport";

			for (const auto& section : obj->sections()) {
				const auto section_name = section.name();
				if (section_name != xport_name) continue;

				const auto data = section.content();

				const auto end = data.data() + data.size();

				std::optional<c4::ast2::symbol> sym;
				for (auto it = parse_one(&sym, data.begin(), end);
				     sym;
				     it = parse_one(&sym, it, end)) {
					const auto let = _ctx.build_let_expression(sym->position(),
					                                           sym.value(),
					                                           nullptr,
					                                           vis);
					let->lift_to_context(_ctx);
					_expressions.push_back(let);
				}
			}
		}

		template<std::integral I>
		constexpr void
		maybe_swap(I& i) noexcept {
			if constexpr (std::endian::native == std::endian::big) {
				i = std::byteswap(i);
			}
		}

		const uint8_t*
		parse_one(std::optional<c4::ast2::symbol>* out,
		          const uint8_t* begin,
		          const uint8_t* end) {
			*out = {};
			if (begin == end) return end;

			if (std::distance(begin, end) < static_cast<ptrdiff_t>(sizeof(uint32_t)))
				throw std::runtime_error("truncated size in export entry");

			const auto sz_bytes = reinterpret_cast<
				const std::array<const uint8_t, sizeof(uint32_t)>*>(begin);
			std::advance(begin, sizeof(uint32_t));

			auto size = std::bit_cast<uint32_t>(*sz_bytes);
			maybe_swap(size);

			if (std::distance(begin, end) < static_cast<ptrdiff_t>(size))
				throw std::runtime_error("truncated symbol name in export entry");

			const std::string_view name(reinterpret_cast<const char*>(begin), size);
			std::advance(begin, size);

			if (std::distance(begin, end) < static_cast<ptrdiff_t>(sizeof(uint32_t)))
				throw std::runtime_error("truncated arity in export entry");

			const auto arity_bytes = reinterpret_cast<
				const std::array<const uint8_t, sizeof(uint32_t)>*>(begin);
			std::advance(begin, sizeof(uint32_t));

			auto arity = std::bit_cast<uint32_t>(*arity_bytes);
			maybe_swap(arity);

			// todo: proper positioning
			*out = c4::ast2::symbol(c4::position::pseudo_position(), name, arity);

			// todo: operator extra byte

			return begin;
		}

		bool
		invalid() const noexcept {
			return error_counter > 1;
		}

		c4::diagnostics_engine& _diag;
		c4::p2::invalid_file_source& _src;
		std::vector<c4::ast2::let_expression*>& _expressions;
		c4::ast2::ast_context& _ctx;
		enum c4::ast2::let_expression::visibility vis;
		int error_counter = 0;
	};
}

std::vector<c4::ast2::let_expression*>
c4::p2::archive_parser::
parse(const enum ast2::let_expression::visibility vis) try {
	_expressions.clear();

	parser_visitor vtor{_diag, _named_src, _expressions, _ctx, vis};
	while (!is_eof(_current)) {
		std::visit(vtor, _current);

		if (vtor.invalid()) break;
		_current = _lexer.next();
	}

	return _expressions;
}
catch (const std::exception& ex) {
	_diag.error(c4::position::invalid_file_position(_named_src),
	            "invalid export structure: cowardly refusing to make sense of it any more: {}",
	            ex.what());
	return {};
}
