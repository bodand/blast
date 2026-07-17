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
 * src/c4rt2/src/c4rt2c/ast2_ir_emitter/resolve_referenced --
 *   
 */

#include <c4/ast2/symbol.hxx>

#include <c4rt2c/ast2_ir_emitter.hxx>

#include <libassert/assert.hpp>

namespace {
	llvm::Function*
	try_get_function_attribute(const c4::ast2::tags::attributable* ref) {
		ASSERT(ref, "ref must not be null");

		const auto attr = ref->attribute_value<llvm::Function*>("function");
		if (!attr) return nullptr;

		return *attr;
	}
}

llvm::Function*
c4rt2c::ast2_ir_emitter::resolve_referenced(const c4::ast2::symbol& sym) {
	if (const auto ref = sym.references()) return try_get_function_attribute(ref);

	const auto symname = _name_manager.global_name(sym);
	if (const auto it = _extlib_functions.find(symname);
		it != _extlib_functions.end())
		return it->second;

	const auto fn = declare_function(symname);
	_extlib_functions.emplace(symname, fn);
	return fn;
}
