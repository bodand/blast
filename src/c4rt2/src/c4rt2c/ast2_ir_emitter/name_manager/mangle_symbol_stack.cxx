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
 * Originally created: 2026-07-17.
 *
 * src/c4rt2/src/c4rt2c/ast2_ir_emitter/name_manager/mangle_symbol_stack --
 *   
 */

#include <numeric>
#include <span>
#include <string>

#include <c4/ast2/symbol.hxx>

#include <c4rt2c/ast2_ir_emitter.hxx>

#include <fmt/format.h>

#include <libassert/assert.hpp>

namespace {
	std::string
	mangle_stack(std::span<const c4::ast2::symbol> symbols) {
		return std::accumulate(
			       symbols.begin(),
			       symbols.end(),
			       fmt::format("q{}", symbols.size()),
			       []<typename TAcc>(TAcc&& acc, const auto& sym) {
				       return std::forward<TAcc>(acc) + sym.mangle();
			       })
		       + "E";
	}

	std::string
	maybe_unmangled_single(const std::span<const c4::ast2::symbol> symbols) {
		const auto& sym = symbols.front();
		if (sym.native()) return sym.mangle();

		return fmt::format("q1N{}E", sym.mangle());
	}
}

std::string
c4rt2c::ast2_ir_emitter::name_manager::mangle_symbol_stack(
	const std::span<const c4::ast2::symbol> symbols
) {
	ASSERT(!symbols.empty(), "symbol stack is empty");

	if (symbols.size() == 1) return maybe_unmangled_single(symbols);
	return mangle_stack(symbols);
}
