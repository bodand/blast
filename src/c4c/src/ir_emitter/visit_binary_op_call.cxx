/* blAST project
 *
 * Copyright (c) 2025 András Bodor <bodand@pm.me>
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
 * Originally created: 2025-04-04.
 *
 * src/c4c/src/ir_emitter/visit_binary_op_call --
 *   Implements the do_visit(binary_op_call&) member function of ir_emitter.
 */

#include <array>
#include <algorithm>

#include <c4c/ir_emitter.hxx>

#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/Transforms/Utils/Cloning.h>
#include <llvm/Transforms/Utils/ValueMapper.h>

#include <libassert/assert.hpp>

void
c4c::ir_emitter::do_visit(const c4::ast2::binary_op_call& obj) {
    // const auto callee = lookup(obj.op());
    // ASSERT(callee,
    //        "callee symbol must be known at the point of call",
    //        obj.op().name(),
    //        obj.op().mangle(),
    //        _promised_symbols);
    //
    // if (!callee->getType()->isPointerTy()) {
    //     ASSERT(callee->getType()->isLabelTy(),
    //            "fn called with value that is not a pointer nor a block",
    //            callee->getName());
    //     const auto bb = cast<llvm::BasicBlock>(callee);
    //     llvm::ValueToValueMapTy vmap;
    //     const auto call_bb = llvm::CloneBasicBlock(bb, vmap, "", _active_function);
    //     auto& val = call_bb->back();
    //     builder.CreateBr(call_bb);
    //
    //     const auto after_bb = build_bblock("after_call");
    //     const auto state = save_state();
    //     builder.SetInsertPoint(call_bb);
    //     builder.CreateBr(after_bb);
    //
    //     last.set_expr(&val);
    //     return;
    // }
    // const auto fn = cast<llvm::Function>(callee);
    //
    // if (const auto ref = obj.op().references()) {
    //     if (const auto context_ref = ref->attribute_value<llvm::Value*>("context")) {
    //         const auto context = *context_ref;
    //         const auto fn_type = fn->getFunctionType();
    //
    //         std::array<llvm::Value*, 3> args{};
    //         args[0] = context;
    //         std::ranges::transform(std::array{&obj.left(), &obj.right()},
    //                                next(args.begin()),
    //                                [this](const c4::ast2::expression* const& arg) {
    //                                    arg->accept_skip_self(*this);
    //                                    return this->last.value;
    //                                });
    //
    //         const auto call = builder.CreateCall(fn_type, callee, args);
    //         last.set_expr(call);
    //
    //         return;
    //     }
    // }
    //
    // std::array<llvm::Value*, 2> args{};
    // std::ranges::transform(std::array{&obj.left(), &obj.right()}, args.begin(),
    //                        [this](const c4::ast2::expression* arg) {
    //                            scope_override_fixer fixer;
    //                            const auto datum_t = llvm::Type::getInt64Ty(context);
    //                            const auto context_t = llvm::PointerType::get(context, 0);
    //                            const auto ptr_t = llvm::PointerType::get(context, 0);
    //
    //                            const auto name_mem = _block_name;
    //                            _block_name = _block_name + ".callarg";
    //
    //                            const auto ctx = create_effective_closure(*arg);
    //
    //                            auto no_ctx_arg = std::array<llvm::Type*, 0>{};
    //                            auto ctx_arg = std::array<llvm::Type*, 1>{context_t};
    //                            std::span<llvm::Type* const> argfn_args =
    //                                    ctx == nullptr
    //                                    ? std::span(no_ctx_arg.begin(), no_ctx_arg.end())
    //                                    : std::span(ctx_arg.begin(), ctx_arg.end());
    //                            const auto argfn_type = llvm::FunctionType::get(datum_t,
    //                                                                            {argfn_args.data(), argfn_args.size()},
    //                                                                            false);
    //                            const auto argfn = llvm::Function::Create(argfn_type,
    //                                                                      llvm::Function::PrivateLinkage,
    //                                                                      _block_name,
    //                                                                      module);
    //                            const auto arg_type = llvm::ArrayType::get(_arg_type, 2);
    //                            const auto mem = builder.CreateAlloca(arg_type, nullptr, {_block_name, ".arg"});
    //                            // const auto data = builder.CreateStructGEP(_arg_type, mem, 1);
    //                            const auto fn_ptr_addr = builder.CreateGEP(ptr_t, mem,
    //                                                                       llvm::ConstantInt::get(
    //                                                                           llvm::Type::getInt32Ty(context), 0));
    //                            const auto ctx_ptr_addr = builder.CreateGEP(ptr_t, mem,
    //                                                                        llvm::ConstantInt::get(
    //                                                                            llvm::Type::getInt32Ty(context), 1));
    //                            builder.CreateStore(argfn, fn_ptr_addr);
    //                            if (ctx) {
    //                                builder.CreateStore(ctx, ctx_ptr_addr);
    //                            }
    //                            else {
    //                                builder.CreateStore(llvm::ConstantPointerNull::get(context_t), ctx_ptr_addr);
    //                            }
    //
    //                            _block_name = name_mem;
    //                            const auto fn = _active_function;
    //                            _active_function = argfn;
    //                            const auto bb = builder.saveIP();
    //                            std::ignore = build_bblock("argbody");
    //
    //                            const auto gep_index_t = llvm::Type::getInt64Ty(context);
    //                            const auto arg_loader_type = llvm::FunctionType::get(
    //                                datum_t, {llvm::PointerType::get(context, 0)}, false);
    //                            const auto arg_loader = module.getFunction("_c4__arg_loader");
    //
    //                            for (std::size_t idx = 0;
    //                                 const auto& referrer : filter_closure_symbols(*arg)) {
    //                                const auto fn_ctx = _active_function->getArg(0);
    //
    //                                const auto ctx_param_ptr = builder.CreateGEP(
    //                                    ptr_t, fn_ctx,
    //                                    llvm::ConstantInt::get(gep_index_t, idx++),
    //                                    llvm::Twine("ctx.", referrer->name()).concat(".addr"));
    //                                const auto ctx_param = builder.CreateLoad(ptr_t, ctx_param_ptr,
    //                                                                          llvm::Twine("ctx.", referrer->name()));
    //
    //                                const auto state = save_state();
    //                                const auto eval_block = llvm::BasicBlock::Create(
    //                                    context,
    //                                    llvm::Twine("ctx.", referrer->name()).concat(".eval"));
    //                                _orphan_blocks.push_back(eval_block);
    //                                builder.SetInsertPoint(eval_block);
    //
    //                                builder.CreateCall(arg_loader_type, arg_loader, {ctx_param}, referrer->mangle());
    //
    //                                if (const auto referee = referrer->references()) fixer.add_symbol(
    //                                    referee, eval_block);
    //                            }
    //
    //                            arg->accept_skip_self(*this);
    //                            builder.CreateRet(last.value);
    //                            builder.restoreIP(bb);
    //                            _active_function = fn;
    //                            return mem;
    //                        });
    //
    // const auto call = builder.CreateCall(fn->getFunctionType(), callee, args);
    // last.set_expr(call);
}
