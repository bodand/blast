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
 * src/c4/include/c4/p2/named_source --
 *   
 */
#ifndef C4_NAMED_SOURCE_HXX
#define C4_NAMED_SOURCE_HXX

#include <filesystem>
#include <string>

namespace c4::p2 {
	struct named_source {
		virtual ~named_source() = default;

		[[nodiscard]] virtual const std::filesystem::path&
		file() const noexcept = 0;

		[[nodiscard]] virtual std::string_view
		file_string() const noexcept = 0;
	};

	struct unknown_source final : named_source {
		[[nodiscard]] const std::filesystem::path&
		file() const noexcept override { return _file; }

		[[nodiscard]] std::string_view
		file_string() const noexcept override { return _file_string; }

	private:
		std::filesystem::path _file{"<unknown>"};
		std::string _file_string{"<unknown>"};
	};

	struct invalid_file_source final : named_source {
		explicit
		invalid_file_source(const std::filesystem::path& file)
			: _file{file} { }

		[[nodiscard]] const std::filesystem::path&
		file() const noexcept override {
			return _file;
		}

		[[nodiscard]] std::string_view
		file_string() const noexcept override {
			if (_file_string.empty()) _file_string = file().string();
			return _file_string;
		}

	private:
		std::filesystem::path _file;
		mutable std::string _file_string;
	};
}

#endif
