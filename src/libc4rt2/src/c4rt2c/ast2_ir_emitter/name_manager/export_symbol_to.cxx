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
#include <concepts>

#include <c4/ast2/let_expression.hxx>

#include <c4rt2c/ast2_ir_emitter.hxx>

#include <libassert/assert.hpp>

static_assert(std::endian::native == std::endian::little
				  || std::endian::native == std::endian::big,
				  "Mixed endian hardware detected: please submit a patch");

namespace {
	template<std::integral I>
	I
	maybe_swap(I i) {
		if constexpr (std::endian::native == std::endian::big) return std::byteswap(i);
		else return i;
	}

	constexpr uint16_t mask_native     = 0b1000'0000'0000'0000;
	constexpr uint16_t mask_operator   = 0b0100'0000'0000'0000;
	constexpr uint16_t mask_left_assoc = 0b0010'0000'0000'0000;
	constexpr uint16_t mask_precedence = 0b0000'0000'0111'1111;
}

void
c4rt2c::ast2_ir_emitter::name_manager::
export_symbol_to(const c4::ast2::let_expression* let, std::vector<uint8_t>& table) {
	const auto sym = let->symbol();
	table.reserve(table.size() + sizeof(uint32_t) + sym.name().size() + sizeof(uint32_t) + 2);

	ASSERT(sym.name().size() < std::numeric_limits<uint32_t>::max(),
	       "Exported symbol names must be shorter than 4 gb",
	       sym.name());

	const uint32_t sym_name_sz = maybe_swap(sym.name().size());
	const uint32_t sym_arity = maybe_swap(sym.base_arity());

	auto sz_bytes = std::bit_cast<std::array<uint8_t, sizeof(sym_name_sz)>>(sym_name_sz);
	auto arity_bytes = std::bit_cast<std::array<uint8_t, sizeof(sym_arity)>>(sym_arity);

	table.insert(table.end(), sz_bytes.begin(), sz_bytes.end());

	const auto* name_data = reinterpret_cast<const uint8_t*>(sym.name().data());
	table.insert(table.end(), name_data, name_data + sym_name_sz);

	table.insert(table.end(), arity_bytes.begin(), arity_bytes.end());

	uint16_t metadata = 0;
	const auto opdata = sym.operator_data();

	metadata |= let->attribute_value<c4::ast2::symbol>("native") ? mask_native : 0;
	metadata |= opdata ? mask_operator : 0;
	if (opdata) {
		metadata |= opdata->left_associative ? mask_left_assoc : 0;
	}
	metadata |= (sym.precedence_like() & mask_precedence);
	metadata = maybe_swap(metadata);

	auto metadata_bytes = std::bit_cast<std::array<uint8_t, sizeof(metadata)>>(metadata);
	table.insert(table.end(), metadata_bytes.begin(), metadata_bytes.end());
}
