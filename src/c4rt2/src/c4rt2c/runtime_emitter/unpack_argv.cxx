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
 * src/c4rt2/src/c4rt2c/runtime_emitter/unpack_argv --
 *   
 */

#include <c4rt2c/runtime_emitter.hxx>

llvm::SmallVector<llvm::Value*, 4>
c4rt2c::runtime_emitter::unpack_argv(llvm::Value* forces,
                                     const std::uint32_t argv_sz) const {
	llvm::SmallVector<llvm::Value*, 4> args;
	args.reserve(argv_sz);

	const auto argv_addr = _builder.CreateStructGEP(
		args_force_t, forces, 3, {forces->getName(), ".argv.addr"});
	const auto argv = _builder.CreateAlignedLoad(
		ptr_t, argv_addr, llvm::Align(8), {forces->getName(), ".argv"});

	std::generate_n(std::back_inserter(args), argv_sz, [&, i=0]() mutable {
		const auto addr = _builder.CreateGEP(ptr_t, argv, _builder.getInt64(i));
		return _builder.CreateAlignedLoad(ptr_t, addr, llvm::Align(8));
	});

	return args;
}
