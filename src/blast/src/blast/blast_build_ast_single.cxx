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
 * Originally created: 2026-08-10.
 *
 * src/blast/src/blast/blast_build_ast_single --
 *   
 */

#include <c4rt3/c4rt.h>

#include <algorithm>
#include <format>
#include <functional>
#include <iostream>
#include <memory>
#include <string_view>
#include <utility>
#include <vector>

#include <gc/gc.h>

#include <c4/array.h>

#include <llvm/ADT/IntrusiveRefCntPtr.h>
#include <llvm/Support/Process.h>

#include <clang/Frontend/ASTUnit.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Tooling/Tooling.h>

#include "../compilation_db.hxx"
#include "../ext-type.hxx"
#include "../gc_box.hxx"
#include "../resource_dir.hxx"

c4_let_native(blast_build_ast_single_)(
	const c4_datum src,
	const c4_datum flags
) try {
	c4_array flags_array;
	c4_datum_get_array(flags, &flags_array);

	char* src_ptr;
	size_t src_sz;
	c4_datum_coerce_string(src, &src_ptr, &src_sz);

	std::vector<std::string> argv;
	argv.reserve(flags_array->len);

	std::transform(
		flags_array->data,
		flags_array->data + flags_array->len,
		std::back_inserter(argv),
		[](const c4_datum arg) {
			char* ptr;
			size_t ptr_sz;
			c4_datum_coerce_string(arg, &ptr, &ptr_sz);
			return std::string(ptr, ptr_sz);
		});

	const auto db = bst::gc_new<bst::compilation_db>(std::move(argv),
		std::filesystem::path(std::string_view(src_ptr, src_sz)));

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
