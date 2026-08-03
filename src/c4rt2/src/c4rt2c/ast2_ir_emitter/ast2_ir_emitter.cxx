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
 * src/c4rt2/src/c4rt2c/ast2_ir_emitter/ast2_ir_emitter --
 *   
 */

#include <c4rt2c/ast2_ir_emitter.hxx>

c4rt2c::ast2_ir_emitter::ast2_ir_emitter(llvm::LLVMContext& context,
                                         llvm::Module& module,
                                         llvm::IRBuilder<>& builder,
                                         runtime_emitter&& rt)
	: _context{context}
	, _module{module}
	, _builder{builder}
	, _di_builder{std::make_unique<llvm::DIBuilder>(module)}
	, _runtime{rt} {
	const auto c4_main_ty = llvm::FunctionType::get(
		llvm::Type::getVoidTy(_context),
		{
			llvm::PointerType::get(_context, 0),
			llvm::PointerType::get(_context, 0)
		}, false
	);
	_c4_main = llvm::Function::Create(
		c4_main_ty,
		llvm::GlobalValue::ExternalLinkage,
		"_c4_main",
		_module
	);
	_c4_main->setCallingConv(llvm::CallingConv::Tail);
	_c4_main->getArg(0)->setName("argv");
	(_mainK = _c4_main->getArg(1))->setName("K");
	_builder.SetInsertPoint(llvm::BasicBlock::Create(_context, "entry", _c4_main));
}
