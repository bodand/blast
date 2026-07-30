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
 * src/c4rt2/src/c4rt2c/ast2_ir_emitter/active_expression --
 *   
 */

#include <c4rt2c/ast2_ir_emitter.hxx>

#include <libassert/assert.hpp>

// TODO: WIP WIP WIP WIP WIP WIP
llvm::DIType*
c4rt2c::ast2_ir_emitter::get_type() {
	if (_ptr_t) return _ptr_t;

	_ptr_t = _di_builder->createBasicType(
		"datum",
		_module.getDataLayout().getPointerSize(),
		llvm::dwarf::DW_AT_object_pointer
	);
	return _ptr_t;
}

llvm::DISubroutineType*
c4rt2c::ast2_ir_emitter::fn_type(const size_t cls_size, const size_t argv_sz) {
	llvm::SmallVector<llvm::Metadata*, 8> params;
	params.reserve(1 + cls_size + argv_sz);

	const auto type = get_type();

	params.push_back(type);
	std::generate_n(std::back_inserter(params), cls_size + argv_sz, [&] { return type; });

	return _di_builder->createSubroutineType(
		_di_builder->getOrCreateTypeArray(params)
	);
}

// TODOEND

const c4::ast2::expression*
c4rt2c::ast2_ir_emitter::active_expression() {
	ASSERT(!_expression_stack.empty(),
	       "empty expression stack has no active expression");
	return _expression_stack.back();
}
