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
 * Originally created: 2026-08-22.
 *
 * src/c4rt2/src/c4rt2c/ast2_ir_emitter/name_manager/export_symbol --
 *   
 */

#include <bit>

#include <c4rt2c/ast2_ir_emitter.hxx>

#include <libassert/assert.hpp>

void
c4rt2c::ast2_ir_emitter::name_manager::
export_symbol_to(const c4::ast2::symbol& symbol, std::vector<uint8_t>& table) {
	table.reserve(table.size() + sizeof(uint32_t) + symbol.name().size() + sizeof(uint32_t));

	ASSERT(symbol.name().size() < std::numeric_limits<uint32_t>::max(),
	       "Exported symbol names must be shorter than 4 gb",
	       symbol.name());

	static_assert(std::endian::native == std::endian::little
	              || std::endian::native == std::endian::big,
	              "Mixed endian hardware detected: please submit a patch");

	uint32_t sym_name_sz = symbol.name().size();
	uint32_t sym_arity = symbol.base_arity();
	if constexpr (std::endian::native == std::endian::big) {
		sym_name_sz = std::byteswap(sym_name_sz);
		sym_arity = std::byteswap(sym_arity);
	}

	auto sz_bytes = std::bit_cast<std::array<uint8_t, sizeof(sym_name_sz)>>(sym_name_sz);
	auto arity_bytes = std::bit_cast<std::array<uint8_t, sizeof(sym_arity)>>(sym_arity);

	table.insert(table.end(), sz_bytes.begin(), sz_bytes.end());

	const auto* name_data = reinterpret_cast<const uint8_t*>(symbol.name().data());
	table.insert(table.end(), name_data, name_data + sym_name_sz);

	table.insert(table.end(), arity_bytes.begin(), arity_bytes.end());
}
