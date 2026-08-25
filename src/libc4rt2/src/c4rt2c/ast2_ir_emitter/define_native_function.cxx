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
 * src/c4rt2/src/c4rt2c/ast2_ir_emitter/define_native_function --
 *   
 */

#include <c4/ast2/symbol.hxx>

#include <c4rt2c/ast2_ir_emitter.hxx>

llvm::Function*
c4rt2c::ast2_ir_emitter::declare_native_function(const c4::ast2::symbol& symbol) {
	llvm::SmallVector<llvm::Type*, 8> args;
	args.reserve(symbol.base_arity());

	const auto ptr_t = llvm::PointerType::get(_context, 0);
	std::generate_n(std::back_inserter(args), symbol.base_arity(), [&]() {
		return ptr_t;
	});

	const auto fn = llvm::Function::Create(llvm::FunctionType::get(ptr_t, args, false),
	                                       llvm::Function::ExternalLinkage,
	                                       symbol.mangle(),
	                                       _module);
	fn->setCallingConv(llvm::CallingConv::C);
	fn->addRetAttr(llvm::Attribute::NoUndef);
	fn->addRetAttr(llvm::Attribute::NonNull);
	fn->addFnAttr(llvm::Attribute::NoUnwind);
	fn->addFnAttr(llvm::Attribute::NoFree);

	std::ranges::for_each(fn->args(), [&](auto& arg) {
		arg.addAttr(llvm::Attribute::NonNull);
		arg.addAttr(llvm::Attribute::NoUndef);
	});

	return fn;
}
