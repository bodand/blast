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
 * Originally created: 2026-08-31.
 *
 * src/c4i/include/c4i/lazy_codegen --
 *   A lazy loading C4 JIT symbol definition generator
 */
#ifndef BLAST_LAZY_CODEGEN_HXX
#define BLAST_LAZY_CODEGEN_HXX

#include <deque>
#include <filesystem>
#include <optional>
#include <memory>
#include <vector>
#include <unordered_map>

#include <c4/ast2/ast_context.hxx>
#include <c4/ast2/fwd.hxx>

#include <llvm/ExecutionEngine/Orc/Core.h>
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>

namespace llvm::orc {
	class IRCompileLayer;
}

namespace c4i {
	struct dependee;

	struct lazy_codegen : llvm::orc::DefinitionGenerator {
		lazy_codegen(c4::diagnostics_engine& diag,
		             c4::p2::source_resolver& resolver,
		             llvm::orc::IRCompileLayer& compile_layer)
			: _diag{diag}
			, _resolver{resolver}
			, _compile_layer{compile_layer} { }

		llvm::Error
		tryToGenerate(llvm::orc::LookupState& LS,
		              llvm::orc::LookupKind K,
		              llvm::orc::JITDylib& JD,
		              llvm::orc::JITDylibLookupFlags JDLookupFlags,
		              const llvm::orc::SymbolLookupSet& LookupSet) override;

		bool
		add_parsed(const std::filesystem::path& path, c4::position from);

	private:
		c4::diagnostics_engine& _diag;
		c4::p2::source_resolver& _resolver;

		llvm::orc::IRCompileLayer& _compile_layer;

		std::vector<std::unique_ptr<dependee>> _parse_holder{};
		std::unordered_map<std::string, dependee*> _symbols{};
	};
}

#endif
