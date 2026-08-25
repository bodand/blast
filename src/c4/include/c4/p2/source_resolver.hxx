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
 * src/c4/include/c4/p2/source_resolver --
 *   
 */
#ifndef BLAST_SOURCE_RESOLVER_HXX
#define BLAST_SOURCE_RESOLVER_HXX

#include <filesystem>
#include <optional>
#include <vector>
#include <c4/archive_file.hxx>

#include <c4/fmt.hxx>

#include <c4/diagnostic.hxx>
#include <c4/source_file.hxx>

namespace c4::p2 {
	struct source_resolver {
		explicit
		source_resolver(diagnostics_engine& diag);

		void
		reset_path() { _search_paths.clear(); }

		void
		push_path(const std::filesystem::path& path) {
			_search_paths.push_back(path);
		}

		std::optional<std::filesystem::path>
		resolve(const source_file& initiator,
		        const position& pos,
		        const bool library,
		        const std::string_view filename) const {
			const auto expected = resolve_expected(initiator, library, filename);

			const auto it = std::ranges::find_if(expected, [&](const auto& path) {
				return exists(path);
			});
			if (it != end(expected)) return *it;

			_diag.error(pos, "could not resolve used file \"{}\" from \"{}\"",
			            filename, _diag.relative(initiator.path()));
			return {};
		}

		source_file
		open(const std::filesystem::path& path) const;

		archive_file
		open_archive(const std::filesystem::path& path) const;

	private:
		std::vector<std::filesystem::path>
		resolve_expected(const source_file& initiator,
		                 const bool library,
		                 std::string_view filename) const {
			if (!library) return {initiator.resolve_source_use(filename)};

			std::vector<std::filesystem::path> paths;
			paths.reserve(_search_paths.size());
			std::ranges::transform(_search_paths, std::back_inserter(paths),
			                       [&](const auto& path) { return path / filename; });
			return paths;
		}

		diagnostics_engine& _diag;
		std::vector<std::filesystem::path> _search_paths;
	};
}

#endif
