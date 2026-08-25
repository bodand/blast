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
 * src/c4/src/archive_file/lex --
 *   
 */

#include <c4/archive_file.hxx>
#include <c4/diagnostic.hxx>

#include <c4/p2/named_source.hxx>

c4::archive_file::
archive_file(diagnostics_engine& diag,
             std::filesystem::path path)
	: _path{std::move(path)} {
	p2::invalid_file_source src{_path};
	if (!exists(_path)) {
		const auto ec = make_error_code(std::errc::no_such_file_or_directory);
		diag.error(position::invalid_file_position(src),
		           "could not open input archive: {}\n",
		           ec.message())
		    .bail("could not open {} for reading", _path.string());
	}

	std::error_code ec;
	_mmap.map(_path.c_str(), 0, mio::map_entire_file, ec);
	if (ec) {
		diag.error(position::invalid_file_position(src),
		           "could not open input archive: {}\n",
		           ec.message())
		    .bail("could not mmap {} for reading", _path.string());
	}
}
