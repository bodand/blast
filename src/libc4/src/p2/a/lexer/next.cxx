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
 * src/c4/src/p2/a/lexer/next --
 *   
 */

#include <cstring>
#include <c4/p2/a/lexer.hxx>

using namespace std::literals;

namespace {
	#pragma pack(push, 1)
	struct archive_header {
		char identifier[16];
		char timestamp[12];
		char owner_id[6];
		char group_id[6];
		char mode[8];
		char size[10];   // !!!!! string not bytes because fuck you
		char trailer[2]; // `\n
	};
	#pragma pack(pop)

	std::pair<std::optional<c4::p2::a::metadata_type>, std::optional<std::string>>
	bsd_filename(archive_header const& hdr,
	             const char*& tail,
	             size_t& file_size) {
		size_t file_name_size;
		auto [ptr, ec] = std::from_chars(hdr.identifier + 3, hdr.identifier + std::size(hdr.identifier),
		                                 file_name_size);
		if (ec != std::errc{}) throw std::runtime_error("invalid archive: invalid file name size");

		std::string name(tail, file_name_size);

		tail += file_name_size;
		file_size -= file_name_size;

		return std::make_pair(std::nullopt, std::move(name));
	}

	std::pair<std::optional<c4::p2::a::metadata_type>, std::optional<std::string>>
	make_sense_of_filename(archive_header const& hdr,
	                       const char*& tail,
	                       size_t& file_size) {
		if (std::memcmp(hdr.identifier, "#1/", 3) == 0) {
			if (hdr.identifier[3] != ' ') return bsd_filename(hdr, tail, file_size);
			// baited, it was a file named #1 in GNU format all along:
			return std::make_pair(std::nullopt, "#1");
		}

		const auto slash = std::ranges::find(hdr.identifier, '/');
		if (slash == std::end(hdr.identifier)) { // bsd short format
			const auto name_end = std::ranges::find(hdr.identifier, ' ');
			std::string name(hdr.identifier, name_end);

			if (name == "__.SYMDEF")
				return std::make_pair(c4::p2::a::metadata_type::BSD_symbol_table,
				                      std::nullopt);

			return std::make_pair(std::nullopt, std::move(name));
		}

		if (slash != hdr.identifier) { // GNU short format
			return std::make_pair(std::nullopt,
			                      std::string(hdr.identifier, slash));
		}

		// we have slash in first position, which still can be three things:
		// * / -> the GNU symbol table of the archive
		// * // -> the GNU string table
		// * /<number> -> long filename named by offset in the string table

		if (std::memcmp(hdr.identifier, "//", 2) == 0) {
			return std::make_pair(c4::p2::a::metadata_type::GNU_string_table,
			                      std::nullopt);
		}

		if (std::memcmp(hdr.identifier, "/ ", 2) == 0) {
			return std::make_pair(c4::p2::a::metadata_type::GNU_symbol_table,
			                      std::nullopt);
		}

		size_t ignore;
		if (const auto [ptr, ec] = std::from_chars(hdr.identifier + 1,
		                                           hdr.identifier + std::size(hdr.identifier), ignore);
			ec == std::errc{}) {
			// We just pass the thing back as-is after having verified it. This
			// is to keep the lexer actually stateless other than the file position
			return std::make_pair(std::nullopt,
			                      std::string(hdr.identifier, ptr));
		}

		throw std::runtime_error(std::format(
			"ar archive string naming convention undecipherable: wtf is '{}'",
			std::string_view(hdr.identifier, std::size(hdr.identifier))));
	}
}

c4::p2::a::tokens::token_type
c4::p2::a::lexer::
next() {
	if (_error_state) return tokens::error{*_error_state};
	if (_data == _end) return tokens::eof{};

	if (static_cast<size_t>(std::distance(_data, _end)) < sizeof(archive_header)) {
		// we can safely recover from this error, so just pretend we reached
		// the end of the blob nicely and in further next-s we can just report eof
		_data = _end;
		return tokens::error{"archive truncated"};
	}

	const auto header_ptr = _data;
	std::advance(_data, sizeof(archive_header));
	const auto hdr = *reinterpret_cast<const archive_header*>(header_ptr);

	if (std::memcmp(hdr.trailer, "`\n", 2)) {
		_error_state = "invalid archive: invalid trailer magic";
		return tokens::error{"invalid archive: invalid trailer magic"};
	}

	size_t file_size;
	if (const auto [ptr, ec] = std::from_chars(hdr.size, hdr.size + std::size(hdr.size), file_size);
		ec != std::errc{}) {
		_error_state = "invalid archive: invalid file size";
		return tokens::error{"invalid archive: invalid file size"};
	}


	if (static_cast<size_t>(std::distance(_data, _end)) < file_size) {
		_error_state = "invalid archive: archive truncated";
		return tokens::error{"invalid archive: archive truncated"};
	}

	const auto [meta, name] = make_sense_of_filename(hdr, _data, file_size);

	const std::span payload{
		reinterpret_cast<const uint8_t*>(_data),
		file_size
	};

	if (file_size % 2 == 1) ++file_size;
	if (static_cast<size_t>(std::distance(_data, _end)) < file_size) {
		_error_state = "invalid archive: truncated padding";
		return tokens::error{"invalid archive: truncated padding"};
	}
	std::advance(_data, file_size);

	if (meta) {
		return tokens::metadata{
			.type = *meta,
			.bytes = payload
		};
	}

	return tokens::object{
		.name = *name,
		.bytes = payload
	};
}
