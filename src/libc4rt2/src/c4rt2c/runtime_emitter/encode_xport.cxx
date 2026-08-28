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
 * Originally created: 2026-08-27.
 *
 * src/libc4rt2/src/c4rt2c/runtime_emitter/encode_xport --
 *   
 */

#include <expected>
#include <span>
#include <vector>

#include <c4rt2c/runtime_emitter.hxx>
#include <libassert/assert.hpp>

std::expected<std::string, std::string>
c4rt2c::
encode_xport(const llvm::Triple& triple,
             std::span<const char> data) {
	std::string assembly;
	if (triple.isOSBinFormatELF()) {
		assembly += ".section .c4xport,\"\",@progbits\n";
	}
	else if (triple.isOSBinFormatMachO()) {
		assembly += ".section __DWARF,__c4xport,regular,debug\n";
	}
	else if (triple.isOSBinFormatCOFF()) {
		assembly += ".section .c4xport,\"dr\"\n";
	}
	else if (triple.isOSBinFormatWasm()) {
		assembly += ".section .custom_section.c4xport,\"\",@\n";
	}
	else if (triple.isOSBinFormatXCOFF()) {
		assembly += ".csect .c4xport[RO]\n";
	}
	else {
		return std::unexpected{
			std::format("unsupported target format: {}: please patch LLVM's "
			            "inline assembly to support your arch",
			            triple.getTriple())
		};
	}
	assembly += ".byte ";

	const auto data_offset = assembly.size();
	assembly.resize(assembly.size() + data.size() * 5 + 1);

	auto to = assembly.data() + data_offset;
	auto begin = assembly.data() + data_offset;
	std::ranges::for_each(data, [&](const unsigned char c) {
		to = std::copy_n("0x", 2, to);
		if (c < 16) *to++ = '0';
		auto [ptr, y] = std::to_chars(to, to + 2, c, 16);
		to = ptr;
		*to++ = ',';
	});
	if (to && to != begin) *--to = '\n';
	DEBUG_ASSERT(to >= begin, "writing stuff only goes forward");

	return assembly;
}
