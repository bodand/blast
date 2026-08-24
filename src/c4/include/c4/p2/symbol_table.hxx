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
 * src/c4/src/p2/symbol_table --
 *   
 */
#ifndef BLAST_SYMBOL_TABLE_HXX
#define BLAST_SYMBOL_TABLE_HXX

#include <deque>
#include <functional>
#include <string_view>

#include <c4/ast2/symbol.hxx>

namespace c4::ast2::tags {
	struct referable;
}

namespace c4::p2 {
	struct parser_symbol {
		std::string_view name;
		ast2::tags::referable* referee;
		unsigned arity;
		unsigned precedence; // Set only on operators
		bool native = false;
		bool right_assoc; // Set only on operators

		parser_symbol(const std::string_view& name_,
		              const unsigned arity_,
		              ast2::tags::referable* referee_,
		              const unsigned precedence_ = 0,
		              const bool right_assoc_ = false) noexcept
			: name{name_}
			, referee{referee_}
			, arity{arity_}
			, precedence{precedence_}
			, right_assoc{right_assoc_} { }

		[[nodiscard]] bool
		is_operator() const noexcept { return precedence != 0; }

		[[nodiscard]] bool
		operator==(const ast2::symbol& sym) const noexcept {
			if (native) return false;
			return sym.name() == name;
		}
	};

	struct symbol_resolution {
		parser_symbol& symbol;
		bool save_in_context;
	};

	struct symbol_table {
		symbol_table() {
			enter_scope();
		}

		std::optional<symbol_resolution>
		find_scoped_symbol(const ast2::symbol& sym);

		std::optional<symbol_resolution>
		find_scoped_symbol_with_arity(const ast2::symbol& sym);

		[[nodiscard]] bool
		root() const noexcept { return _scope_symbol_size.size() == 1; }

		void
		enter_scope();

		void
		leave_scope();

		parser_symbol*
		find_operator(std::string_view name, unsigned arity);

		parser_symbol*
		find_infix_operator(const std::string_view name) {
			return find_operator(name, 2);
		}

		parser_symbol*
		find_prefix_operator(const std::string_view name) {
			return find_operator(name, 1);
		}

		parser_symbol&
		declare(std::string_view symbol,
		        unsigned arity,
		        ast2::tags::referable* referee = nullptr,
		        unsigned precedence = 0,
		        bool right_assoc = false);

	private:
		std::vector<unsigned> _scope_symbol_size;
		std::deque<parser_symbol> _scope_symbols;
	};
}

#endif
