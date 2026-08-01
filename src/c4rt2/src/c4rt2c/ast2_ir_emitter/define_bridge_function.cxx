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
 * Originally created: 2026-07-31.
 *
 * src/c4rt2/src/c4rt2c/ast2_ir_emitter/define_bridge_function --
 *   
 */

#include <cctype>

#include <c4/ast2/let_expression.hxx>
#include <c4rt2c/ast2_ir_emitter.hxx>
#include <c4rt2c/scoped_scope.hxx>

#include <libassert/assert.hpp>

void
c4rt2c::ast2_ir_emitter::define_bridge_function(
	const c4::ast2::let_expression& let
) {
	auto trampoline_name = _name_manager.mangle_symbol(let.symbol());
	trampoline_name[0] = static_cast<char>(std::toupper(trampoline_name[0]));
	const auto trampoline = declare_function(trampoline_name);
	trampoline->setLinkage(llvm::GlobalValue::InternalLinkage);

	const auto fn = let.attribute_value<llvm::Function*>("function");
	ASSERT(fn, "function attribute must not be null", let.symbol());

	const auto native_sym = let.attribute_value<c4::ast2::symbol>("native");
	ASSERT(native_sym, "native attribute not set", let.symbol());

	const auto native_fn = let.attribute_value<llvm::Function*>("native-function");
	ASSERT(native_fn, "native-function attribute not set", let.symbol());

	// Define trampoline to call native function after unrolling argument count
	// then jumping forward to K
	{
		scoped_scope scope(_builder);
		_builder.SetInsertPoint(define(trampoline));
		const auto forces = trampoline->getArg(0);
		const auto K = trampoline->getArg(1);

		const auto args = _runtime.unpack_argv(forces,
		                                       native_sym->base_arity());
		const auto ret = _builder.CreateCall(*native_fn, args);

		_runtime.continue_with(K, ret);
	}

	// normal C4-side of bridge
	const auto argv = (*fn)->getArg(0);
	const auto K = (*fn)->getArg(1);

	const auto forces = _runtime.make_forces_blob(
		argv,
		native_sym->base_arity(),
		trampoline);
	_runtime.force_args(forces, K);
}
