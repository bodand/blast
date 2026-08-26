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
 * Originally created: 2026-08-02.
 *
 * src/c4rt2/src/c4rt2c/runtime_fn/runtime_fn_debug_info_builder --
 *   
 */

#include <ranges>
#include <c4rt2c/runtime_fn.hxx>
#include <libassert/assert.hpp>

#include <llvm/IR/DIBuilder.h>


c4rt2c::runtime_fn_debug_info_builder::~runtime_fn_debug_info_builder() {
	if (_target->decl_only()) return;

	std::vector<llvm::Metadata*> types;
	types.reserve(1 + _args.size());

	types.emplace_back(_ret_type);
	std::ranges::transform(_args, std::back_inserter(types), [](const auto& var) {
		return var.type;
	});

	const auto fn_type = _dib->createSubroutineType(
		_dib->getOrCreateTypeArray(types));

	const auto sub = _dib->createFunction(_scope, _name, _name, _file, _line,
	                                      fn_type, _scope_line,
	                                      llvm::DINode::FlagPrototyped,
	                                      llvm::DISubprogram::SPFlagDefinition);

	std::vector<llvm::DILocalVariable*> di_args;
	di_args.reserve(_args.size());
	std::ranges::transform(_args, std::back_inserter(di_args), [&, i = 0u](const auto& var) mutable {
		constexpr bool always_preserve = true;
		return _dib->createParameterVariable(sub, var.name, ++i, _file, 0, var.type, always_preserve);
	});

	const auto fn = _target->fn;
	auto& ctx = fn->getContext();
	llvm::IRBuilder<> builder(ctx);

	fn->setSubprogram(sub);
	_target->_dib = _dib;

	std::vector<llvm::Argument*> unfucked_args(fn->arg_size());
	std::transform(fn->arg_begin(), fn->arg_end(), unfucked_args.begin(),
	               [](llvm::Argument& arg) { return &arg; });

	std::ranges::for_each(
		std::ranges::views::zip(unfucked_args, di_args, _args),
		[&](const auto& p) {
			llvm::Argument* const arg = std::get<0>(p);
			llvm::DILocalVariable* const di_arg = std::get<1>(p);
			const struct arg local_arg = std::get<2>(p);

			const auto dbg_loc = llvm::DILocation::get(ctx, 0, 0, sub);

			const auto insert = _target->entry();
			const auto insert_ptr = insert->getFirstInsertionPt();

			arg->setName(local_arg.name);
			builder.SetInsertPoint(insert_ptr);
			builder.SetCurrentDebugLocation(dbg_loc);

			const auto addr = builder.CreateAlloca(arg->getType(), nullptr,
			                                       {local_arg.name, ".addr"});
			builder.CreateStore(arg, addr);
			_dib->insertDeclare(addr, di_arg, local_arg.expr, dbg_loc, insert_ptr);
		});
}
