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
 * Originally created: 2025-03-03.
 *
 * src/c4c/src/c4_runtime_emitter --
 *   
 */

#include <c4c/c4_runtime_emitter.hxx>

#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>

#include <libassert/assert.hpp>
#include <llvm/IR/IRBuilder.h>

#define CAT(x, y) CAT_I(x, y)
#define CAT_I(x, y) x##y

#define STR(x) STR_I(x)
#define STR_I(x) #x

#define DEF_RTF_TYPE(sym, ret, ...) const auto CAT(sym, _ft) = FunctionType::get(ret, {__VA_ARGS__}, false)
#define DEF_RTF(idx, sym) _symbol_map[idx] = Function::Create(CAT(sym, _ft), GlobalValue::ExternalLinkage, STR(CAT(c4rt_, sym)), module)

#define DEF_TYPED(idx, sym, ret, ...) DEF_RTF_TYPE(sym, ret, __VA_ARGS__); DEF_RTF(idx, sym)

#define DEF_CTOR_FT(from) DEF_RTF_TYPE(CAT(datum_from_, from), datum_t, CAT(from, _t))
#define DEF_CTOR(idx, type) DEF_CTOR_FT(type); DEF_RTF(idx, CAT(datum_from_, type))

c4c::c4_runtime_emitter::c4_runtime_emitter(llvm::LLVMContext& context,
                                            llvm::Module& module)
    : _context(context) {
    using namespace llvm;
    const auto datum_t = Type::getInt64Ty(context);
    const auto int64_t = Type::getInt64Ty(context);
    const auto int32_t = Type::getInt32Ty(context);
    const auto double_t = Type::getDoubleTy(context);
    const auto string_t = Type::getInt8Ty(context)->getPointerTo();
    const auto void_t = Type::getVoidTy(context);
    const auto datum_ptr_t = datum_t->getPointerTo();
    const auto anyptr_t = PointerType::get(context, 0);

    // constructors
    DEF_CTOR(c4rt_symbol::DatumFromInt32, int32);
    DEF_CTOR(c4rt_symbol::DatumFromInt64, int64);
    DEF_CTOR(c4rt_symbol::DatumFromDouble, double);
    DEF_CTOR(c4rt_symbol::DatumFromString, string);
    const auto datum_from_proc_ft = FunctionType::get(datum_t, {anyptr_t}, false);
    DEF_RTF(c4rt_symbol::DatumFromProc, datum_from_proc);

    // destructor
    DEF_TYPED(c4rt_symbol::DatumFree, datum_free, void_t, datum_t);

    // retrievals
    DEF_TYPED(c4rt_symbol::DatumGetInt32, datum_get_int32, int32_t, datum_t);
    DEF_TYPED(c4rt_symbol::DatumCoerceInt32, datum_coerce_int32, int32_t, datum_t);
    DEF_TYPED(c4rt_symbol::DatumGetInt64, datum_get_int64, int64_t, datum_t);
    DEF_TYPED(c4rt_symbol::DatumCoerceInt64, datum_coerce_int64, int64_t, datum_t);
    DEF_TYPED(c4rt_symbol::DatumGetDouble, datum_get_double, double_t, datum_t);
    DEF_TYPED(c4rt_symbol::DatumCoerceDouble, datum_coerce_double, double_t, datum_t);
    DEF_TYPED(c4rt_symbol::DatumGetString, datum_get_string, string_t, datum_t);
    DEF_TYPED(c4rt_symbol::DatumCoerceString, datum_coerce_string, string_t, datum_t);
}

llvm::Value*
c4c::c4_runtime_emitter::emit_rt_call(const c4rt_symbol sym,
                                      llvm::IRBuilder<>& builder,
                                      const llvm::ArrayRef<llvm::Value*> args) {
    const auto it = _symbol_map.find(sym);
    ASSERT(it != _symbol_map.end(), "invalid c4 runtime symbol", sym);

    return builder.CreateCall(it->second, args);
}

llvm::FunctionType*
c4c::c4_runtime_emitter::get_c4_funtype(const unsigned arity,
                                        const bool context) const {
    const auto datum_t = llvm::Type::getInt64Ty(_context);
    const auto ctx_t = datum_t->getPointerTo();

    const auto effective_arity = arity + (context ? 1 : 0);

    std::vector<llvm::Type*> args(effective_arity);
    auto args_begin = args.begin();
    if (context) {
        *args_begin = ctx_t;
        ++args_begin;
    }
    std::generate_n(args_begin, arity, [&datum_t] { return datum_t; });

    return llvm::FunctionType::get(datum_t, args, false);
}
