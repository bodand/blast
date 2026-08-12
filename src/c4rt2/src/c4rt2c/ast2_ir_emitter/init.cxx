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
 * Originally created: 2026-07-30.
 *
 * src/c4rt2/src/c4rt2c/init --
 *   
 */

#include <filesystem>

#include <c4rt2c/ast2_ir_emitter.hxx>

#include <llvm/IR/DIBuilder.h>
#include <llvm/IR/Module.h>

namespace fs = std::filesystem;
namespace dwf = llvm::dwarf;

void
c4rt2c::ast2_ir_emitter::init(const fs::path& fname) {
	const auto filename = fname.filename().string();
	const auto directory = fname.parent_path().string();

	_module.addModuleFlag(llvm::Module::Warning, "Debug Info Version", llvm::DEBUG_METADATA_VERSION);
	_cu = _di_builder->createCompileUnit(
		dwf::DW_LANG_Haskell, // XXX is this ok?
		_di_builder->createFile(filename, directory),
		"c4c",
		false,
		"",
		0
	);

	_runtime.init(_di_builder.get(), _cu);

	_file = _di_builder->createFile(
		_cu->getFilename(),
		_cu->getDirectory());

	const auto sub = _di_builder->createFunction(
		_file,
		"_c4_main",
		"_c4_main",
		_file,
		0,
		_runtime.fn_type(0, 0),
		0,
		llvm::DINode::FlagArtificial,
		llvm::DISubprogram::SPFlagDefinition
	);
	_c4_main->setSubprogram(sub);
	_builder.SetCurrentDebugLocation(
		llvm::DILocation::get(_context, 0, 0, sub));
}
