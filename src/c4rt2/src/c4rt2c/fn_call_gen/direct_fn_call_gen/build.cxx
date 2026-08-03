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
 * Originally created: 2026-08-03.
 *
 * src/c4rt2/src/c4rt2c/fn_call_gen/direct_fn_call_gen/build --
 *   
 */

#include <algorithm>

#include <c4/ast2/expression.hxx>

#include <c4/tags/referable.hxx>

#include <c4rt2c/ast2_ir_emitter.hxx>
#include <c4rt2c/fn_call_gen.hxx>
#include <c4rt2c/runtime_emitter.hxx>

#include <libassert/assert.hpp>

llvm::Value*
c4rt2c::direct_fn_call_gen::
build(ast2_ir_emitter& ir, const c4::ast2::expression& expr) {
	_fn = resolve_symbol_value(_symbol);
	ASSERT(_fn, "symbol is not defined", _symbol);

	// this for global variables
	if (refer()->attribute_value<bool>("needs-loading?")) {
		return _builder.CreateLoad(_builder.getPtrTy(0), _fn,
		                           {_symbol.name(), ".load"});
	}

	llvm::SmallVector<llvm::Value*, 4> closure_over;
	load_closures(std::back_inserter(closure_over));
	_argv_sz = closure_over.size() + args_sz();
	ASSERT(_argv_sz < UINT32_MAX, "way too much arguments for thunk");

	const auto layout = ir.module().getDataLayout();
	if (!refer()->thunk()) {
		_argv = _rt.allocate_array(_argv_sz, layout.getPointerSize(0));
		_argv->setName({_symbol.name(), ".argv"});

		std::ranges::for_each(closure_over, [&, i=0](const auto& val) mutable {
			const auto addr = _builder.CreateGEP(
				_builder.getPtrTy(0),
				_argv,
				_builder.getInt32(i++));
			_builder.CreateStore(val, addr);
		});
	}

	finalize(_fn, _argv, _argv_sz, expr);

	std::ranges::for_each(args(), [&, i=_closure_args_sz](const auto& arg) mutable {
		ASSERT(_argv, "argv array must be defined when arguments are passed");

		arg->accept(ir);

		const auto addr = _builder.CreateGEP(
			_builder.getPtrTy(0),
			_argv,
			_builder.getInt64(i++));

		auto value = arg->template attribute_value<llvm::Value*>("value");
		ASSERT(value, "argument didn't get defined to value",
				 _symbol.name(), arg);
		_builder.CreateStore(*value, addr);
	});

	return _fn;
}
