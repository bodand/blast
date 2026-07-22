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
 * src/c4rt2/src/c4rt2c/ast2_ir_emitter/resolve_symbol --
 *   
 */

#include <iostream>
#include <c4/ast2/symbol.hxx>

#include <c4rt2c/ast2_ir_emitter.hxx>

#include <libassert/assert.hpp>

namespace {
	llvm::Value*
	try_get_value_attribute(const c4::ast2::symbol& sym) {
		const auto ref = sym.references();
		if (!ref) return nullptr;

		const auto attr = ref->attribute_value<llvm::Value*>("value");
		if (!attr) return nullptr;

		return *attr;
	}
}

llvm::Value*
c4rt2c::ast2_ir_emitter::resolve_symbol(const c4::ast2::symbol& sym) const {
	if (const auto value = try_get_value_attribute(sym)) return value;

	const auto gsym = name_manager::global_name(sym);
	if (const auto value = _module.getNamedValue(gsym)) return value;

	std::clog << "creating " << gsym << std::endl;
	return llvm::Function::Create(_runtime.function_type(), llvm::GlobalValue::ExternalLinkage, gsym, _module);
}
