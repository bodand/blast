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
 * src/c4rt2/src/c4rt2c/ast2_ir_emitter/define_global_var --
 *   
 */

#include <c4/ast2/expression.hxx>
#include <c4/ast2/let_expression.hxx>

#include <c4rt2c/ast2_ir_emitter.hxx>

#include <libassert/assert.hpp>

llvm::GlobalVariable*
c4rt2c::ast2_ir_emitter::define_global_var(const c4::ast2::let_expression* gsym) {
	const auto ptr_t = llvm::PointerType::get(_context, 0);

	// _module takes ownership of this, no need to fret
	const auto var = new llvm::GlobalVariable(_module, ptr_t, false,
	                                          llvm::GlobalValue::PrivateLinkage,
	                                          llvm::ConstantPointerNull::get(ptr_t));
	const auto var_name = _name_manager.mangle_symbol(gsym->symbol());
	var->setName(var_name);

	gsym->value().accept(*this);
	const auto val = gsym->value().attribute_value<llvm::Value*>("value");
	ASSERT(val, "symbol didn't get defined to value", var_name, gsym);

	_builder.CreateStore(*val, var);
	return var;
}
