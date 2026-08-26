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
 * Originally created: 2026-07-17.
 *
 * src/c4rt2/src/c4rt2c/ast2_ir_emitter/finalize --
 *   
 */

#include <llvm/IR/DIBuilder.h>
#include <llvm/Transforms/Utils/ModuleUtils.h>

#include <c4rt2c/ast2_ir_emitter.hxx>
#include <llvm/IR/Module.h>

void
c4rt2c::ast2_ir_emitter::finalize() {
	_seq_builder.build(_builder, _runtime, _mainK);
	if (!_builder.GetInsertBlock()->getTerminator()) _builder.CreateRetVoid();

	std::ranges::sort(_export_table);
	const auto [from, to] = std::ranges::unique(_export_table);
	_export_table.erase(from, to);

	const auto total_size = std::accumulate(_export_table.begin(),
	                                        _export_table.end(),
	                                        std::size_t{},
	                                        [](const auto agg, const auto& elem) {
		                                        return agg + elem.size();
	                                        });
	std::vector<uint8_t> merged_table;
	merged_table.reserve(total_size);
	std::ranges::for_each(_export_table, [&merged_table](const auto& enty) {
		merged_table.insert(merged_table.end(), enty.begin(), enty.end());
	});

	const auto exports = llvm::ConstantDataArray::get(_context, merged_table);
	const auto exports_global =
			// do not fret: the module yoinks ownership
			new llvm::GlobalVariable(_module,
			                         exports->getType(),
			                         true,
			                         llvm::GlobalValue::PrivateLinkage,
			                         exports,
			                         "_c4_exports_table");

	if (const auto& triple = _module.getTargetTriple();
		triple.isOSBinFormatMachO()) {
		exports_global->setSection("__DWARF,__c4xport");
	}
	else {
		exports_global->setSection(".c4xport");
	}

	llvm::appendToUsed(_module, exports_global);

	_di_builder->finalize();
}
