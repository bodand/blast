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
 * Originally created: 2026-08-12.
 *
 * src/blast/src/compilation_db/compilation_db --
 *   
 */

#include "../compilation_db.hxx"

#include <clang/Tooling/JSONCompilationDatabase.h>

#include "../resource_dir.hxx"

namespace fs = std::filesystem;

bst::compilation_db::
compilation_db(const fs::path& path)
	: _resource_dir_flag{std::format("-resource-dir={}", resource_dir())} {
	std::string error;

	if (!exists(path)) throw std::runtime_error("compilation database not found");
	_db = clang::tooling::JSONCompilationDatabase::loadFromFile(
		path.c_str(), error,
		clang::tooling::JSONCommandLineSyntax::AutoDetect);
	if (!_db) throw std::runtime_error(error);

	const auto str_files = _db->getAllFiles();
	_files.reserve(str_files.size());
	std::ranges::transform(str_files, std::back_inserter(_files),
	                       [](const auto& file) { return fs::path(file); });
}

bst::compilation_db::
compilation_db(std::vector<std::string>&& args, const fs::path& file)
	: _resource_dir_flag{std::format("-resource-dir={}", resource_dir())}
	, _files{file} {
	_db = std::make_unique<clang::tooling::FixedCompilationDatabase>(".", args);
}
