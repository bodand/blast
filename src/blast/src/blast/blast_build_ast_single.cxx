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
#include <iostream>
#include <string_view>
#include <vector>

#include <gc/gc.h>

#include <c4/array.h>

#include <llvm/ADT/IntrusiveRefCntPtr.h>

#include <clang/Frontend/CompilerInstance.h>
#include <clang/Tooling/Tooling.h>

#include "../ext-type.hxx"
#include "../gc_box.hxx"
#include "../resource_dir.hxx"

c4_let_native(blast_build_ast_single)(const c4_datum src,
                                      const c4_datum flags) {
	c4_array flags_array;
	c4_datum_get_array(flags, &flags_array);

	char* src_ptr;
	size_t src_sz;
	c4_datum_coerce_string(src, &src_ptr, &src_sz);

	std::vector<std::string> argv;
	argv.reserve(3 + flags_array->len + 1);
	argv.emplace_back("blast"); // XXX -- proper argv0
	argv.emplace_back("-fsyntax-only");
	argv.emplace_back(std::format("-resource-dir={}", resource_dir()));

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
	argv.emplace_back(src_ptr, src_sz);

	std::vector<const char*> argv_ptrs;
	argv_ptrs.reserve(argv.size());
	std::ranges::transform(argv, std::back_inserter(argv_ptrs),
	                       std::mem_fn(&std::string::c_str));

	// XXX -- implement custom diagnostics handling
	const auto diag_opts = llvm::makeIntrusiveRefCnt<clang::DiagnosticOptions>();
	const auto diags = clang::CompilerInstance::createDiagnostics(diag_opts.get());

	const auto pch = std::make_shared<clang::PCHContainerOperations>();
	auto unit = clang::ASTUnit::LoadFromCommandLine(
		argv_ptrs.data(),
		argv_ptrs.data() + argv_ptrs.size(),
		pch,
		diags,
		resource_dir()
	);
	const auto gc_unit = bst::gc_box(std::move(unit));

	c4_datum out;
	c4_datum_from_ast_unit(gc_unit, &out);
	return out;
}
