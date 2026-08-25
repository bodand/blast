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
 * src/blast/src/blast/blast_build_from_compile_commands --
 *   
 */

#include <c4rt3/c4rt.h>

#include <c4/array.h>

#include <filesystem>
#include <iostream>

#include <clang/Frontend/ASTUnit.h>
#include <clang/Tooling/JSONCompilationDatabase.h>
#include <clang/Tooling/Tooling.h>

#include "../compilation_db.hxx"
#include "../ext-type.hxx"
#include "../gc_box.hxx"
#include "../resource_dir.hxx"

namespace fs = std::filesystem;

c4_let_native(blast_build_from_compile_commands)(
	const c4_datum compile_commands_json
) try {
	char* json_path_str;
	size_t json_path_sz;
	c4_datum_coerce_string(compile_commands_json, &json_path_str, &json_path_sz);
	const auto json_path_view = std::string_view(json_path_str, json_path_sz);

	const fs::path json_path(json_path_view);
	const auto db = bst::gc_new<bst::compilation_db>(json_path);

	c4_datum out;
	c4_datum_from_db(db, &out);
	return out;
}
catch (const std::exception& e) {
	std::cerr << "blast: fatal: " << e.what() << std::endl;
	c4_datum nil;
	c4_datum_from_nil(&nil);
	return nil;
}
