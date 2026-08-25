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
 * src/c4/include/c4/p2/a/lexer --
 *   The archive lexer returns blobs of object files as it chews itself through
 *   an archive file.
 */
#ifndef BLAST_LEXER_HXX
#define BLAST_LEXER_HXX

#include <filesystem>
#include <optional>
#include <span>
#include <string_view>
#include <variant>

namespace c4::p2::a {
	enum class metadata_type {
		GNU_string_table,
		GNU_symbol_table,
		BSD_symbol_table,
	};

	// not in external file, because it is quite small
	namespace tokens {
		struct object {
			std::string name = "undef";
			std::span<const uint8_t> bytes;
		};

		struct metadata {
			metadata_type type;
			std::span<const uint8_t> bytes;
		};

		struct eof { };

		struct error {
			std::string message;
		};

		using token_type = std::variant<
			metadata,
			object,
			eof,
			error
		>;
	}

	struct lexer {
		lexer(std::filesystem::path const& path,
		      const char* begin,
		      const char* end);

		tokens::token_type
		next();

	private:
		const char* const _end;
		const char* _data;
		std::optional<std::string> _error_state;
	};
}

#endif
