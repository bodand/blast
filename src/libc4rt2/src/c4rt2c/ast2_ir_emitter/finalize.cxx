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
#include <llvm/IR/Function.h>
#include <llvm/Transforms/Utils/ModuleUtils.h>

#include <c4rt2c/ast2_ir_emitter.hxx>
#include <llvm/IR/Module.h>

void
c4rt2c::ast2_ir_emitter::
finalize(const bool want_main,
         const bool want_lib) {
	if (want_main) {
		// use explicitly asked for main (=exec entry), give it to them
		_c4_main->setName("_c4_main");
		_seq_builder.build(_builder, _runtime, _mainK);
		if (!_builder.GetInsertBlock()->getTerminator()) _builder.CreateRetVoid();
	}
	else if (want_lib) {
		// use explicitly asked for library (=lib entry), give it to them
		_seq_builder.build(_builder, _runtime, _mainK);
		if (!_builder.GetInsertBlock()->getTerminator()) _builder.CreateRetVoid();
	}
	else {
		// we need to figure out what the user meant from vibes
		const bool did_stuff_in_entry = !_c4_main->getEntryBlock().empty()
		                                || !_seq_builder.empty();
		const bool defined_a_function = _has_local_let;

		if (did_stuff_in_entry) {
			// if there is stuff in the entry, we need to keep it anyhow. At some
			// point in the future, library entry points may become non-dead code
			_c4_main->setName("_c4_main");
			_seq_builder.build(_builder, _runtime, _mainK);
			if (!_builder.GetInsertBlock()->getTerminator()) _builder.CreateRetVoid();
		}
		else {
			// we don't have anything in the entry point, which would suggest lib
			// but if don't have functions, an empty file is an exec
			if (defined_a_function) {
				// we have a function, but nothing in the entry point -- this means
				// the most likely use is a library, so we can just drop the empty
				// library entry point
				_c4_main->eraseFromParent();
			}
			else {
				// and so it is an exec, we need to finalize the entrypoint which
				// will ultimately do absolutely nothing, but alas
				_c4_main->setName("_c4_main");
				_seq_builder.build(_builder, _runtime, _mainK);
				if (!_builder.GetInsertBlock()->getTerminator()) _builder.CreateRetVoid();
			}
		}
	}

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

	const auto assembly = encode_xport(_module.getTargetTriple(), merged_table);
	if (!assembly) throw std::runtime_error(assembly.error());
	_module.appendModuleInlineAsm(*assembly);

	_di_builder->finalize();
}
