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
 * Originally created: 2026-07-19.
 *
 * src/c4rt2/src/c4rt2c/ast2_ir_emitter/thunked_symbol --
 *   
 */

#include <c4/ast2/symbol.hxx>

#include <c4rt2c/ast2_ir_emitter.hxx>

#include <llvm/IR/IRBuilder.h>

#include <libassert/assert.hpp>

std::pair<bool, llvm::Value*>
c4rt2c::ast2_ir_emitter::thunked_symbol(const c4::ast2::symbol& sym) const {
	const auto ref = sym.references();
	if (!ref) {
		auto fn = _module.getOrInsertFunction(name_manager::global_name(sym),
		                                      _runtime.function_type());
		auto thunk = _runtime.make_thunk(fn.getCallee());
		thunk->setName({sym.name(), ".thunk"});
		return {true, thunk};
	}

	const auto fn_attr = ref->attribute_value<llvm::Function*>("function");
	const auto val_attr = ref->attribute_value<llvm::Value*>("value");

	if (ref->introduces_function()) {
		ASSERT(fn_attr, "function attribute must not be null", ref, sym.name(), sym.base_arity());
		if (ref->thunk()) return {false, *fn_attr};

		auto thunk = _runtime.make_thunk(*fn_attr);
		thunk->setName({sym.name(), ".thunk"});
		return {true, thunk};
	}

	// values are always thunks
	ASSERT(val_attr, "value attribute must not be null", ref, sym.name(), sym.base_arity());
	return {false, *val_attr};
}
