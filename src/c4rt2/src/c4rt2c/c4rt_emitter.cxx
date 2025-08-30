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
 * Originally created: 2025-08-08.
 *
 * src/c4rt2/src/c4rt2c/c4rt_emitter --
 *   
 */

#include <c4rt2/c4rt.h>

#include <llvm/Linker/Linker.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/SourceMgr.h>

#include <c4rt2c/c4rt_emitter.hxx>
#include <catch2/internal/catch_void_type.hpp>

#include <libassert/assert.hpp>

#include "../../include/c4rt2/c4rt_package_versions.h"

c4rt2c::c4_rt2_emitter::
c4_rt2_emitter(llvm::Module& module, llvm::IRBuilder<>* builder)
    : _c4rt_datum_type{llvm::Type::getInt64Ty(builder->getContext())}
    , _c4rt_package_type{llvm::PointerType::get(builder->getContext(), 0)}
    , _builder{builder}
    , _module{module} {
    const auto int64_t = llvm::Type::getInt64Ty(_builder->getContext());
    const auto int32_t = llvm::Type::getInt32Ty(_builder->getContext());
    const auto int16_t = llvm::Type::getInt16Ty(_builder->getContext());
    _package_struct_type = llvm::StructType::create(_module.getContext(), {int16_t, int16_t, int16_t, int16_t, int64_t},
                                                    "c4rt_package_v1_t",
                                                    true);

    const auto type_t = llvm::Type::getInt32Ty(_builder->getContext());
    const auto ptr_t = llvm::PointerType::get(_builder->getContext(), 0);
    const auto void_t = llvm::Type::getVoidTy(_builder->getContext());

    declare_rt_function("c4rt_datum_from_static_ptr", _c4rt_datum_type, type_t, ptr_t);
    declare_rt_function("c4rt_datum_from_function", _c4rt_datum_type, ptr_t, int16_t, ptr_t, int64_t);
    declare_rt_function("c4rt_datum_from_closure", _c4rt_datum_type, ptr_t, int16_t, ptr_t, int64_t, ptr_t, int64_t);
    declare_rt_function("c4rt_datum_evaluate", _c4rt_datum_type, _c4rt_datum_type, ptr_t);
    declare_rt_function("c4rt_package_init_from_result", void_t, ptr_t, _c4rt_datum_type);
    declare_rt_function("c4rt_package_init_from_function", void_t, ptr_t, ptr_t, ptr_t, int32_t);
    declare_rt_function("c4rt_package_init_from_closure", void_t,
                        ptr_t, /*pkg*/
                        ptr_t, /*fn*/
                        ptr_t, /*ctx*/ int32_t, /*ctx_sz*/
                        ptr_t, /*fn_data*/ int32_t /*fn_data_sz*/);
    declare_rt_function("c4rt_package_evaluate", _c4rt_datum_type, ptr_t);
}

llvm::Value*
c4rt2c::c4_rt2_emitter::emit_datum_from_static_ptr(llvm::Value* type, llvm::Value* ptr) const {
    const auto type_t = llvm::Type::getInt32Ty(_builder->getContext());
    const auto ptr_t = llvm::PointerType::get(_builder->getContext(), 0);

    llvm::FunctionType* datum_from_static_ptr_ft = llvm::FunctionType::get(_c4rt_datum_type, {type_t, ptr_t}, false);
    const auto fn = _module.getFunction("c4rt_datum_from_static_ptr");
    ASSERT(fn, "c4rt_datum_from_static_ptr must be defined in module");

    return _builder->CreateCall(datum_from_static_ptr_ft, fn, {type, ptr});
}

llvm::Value*
c4rt2c::c4_rt2_emitter::emit_datum_from_function(llvm::Value* func,
                                                 llvm::Value* arity,
                                                 llvm::Value* fn_data,
                                                 llvm::Value* fn_data_sz) const {
    const auto ptr_t = llvm::PointerType::get(_builder->getContext(), 0);
    const auto int16_t = llvm::Type::getInt16Ty(_builder->getContext());
    const auto int64_t = llvm::Type::getInt64Ty(_builder->getContext());

    llvm::FunctionCallee fn;
    get_rt_function(&fn, "c4rt_datum_from_function", _c4rt_datum_type, ptr_t, int16_t, ptr_t, int64_t);
    return _builder->CreateCall(fn, {func, arity, fn_data, fn_data_sz});
}

llvm::Value*
c4rt2c::c4_rt2_emitter::emit_datum_from_closure(llvm::Value* func,
                                                llvm::Value* arity,
                                                llvm::Value* ctx,
                                                llvm::Value* ctx_sz,
                                                llvm::Value* fn_data,
                                                llvm::Value* fn_data_sz) const {
    const auto ptr_t = llvm::PointerType::get(_builder->getContext(), 0);
    const auto int16_t = llvm::Type::getInt16Ty(_builder->getContext());
    const auto int64_t = llvm::Type::getInt64Ty(_builder->getContext());

    llvm::FunctionCallee fn;
    get_rt_function(&fn, "c4rt_datum_from_closure", _c4rt_datum_type, ptr_t, int16_t, ptr_t, int64_t, ptr_t, int64_t);
    return _builder->CreateCall(fn, {func, arity, ctx, ctx_sz, fn_data, fn_data_sz});
}

llvm::Value*
c4rt2c::c4_rt2_emitter::emit_datum_evaluate(llvm::Value* datum, const std::span<llvm::Value*> args) const {
    const auto ptr_t = llvm::PointerType::get(_builder->getContext(), 0);
    const auto index_type = _module.getDataLayout().getIndexType(_module.getContext(), 0);

    llvm::Value* pkg_args = llvm::ConstantPointerNull::get(ptr_t);
    if (!args.empty()) {
        const auto args_sz_val = llvm::ConstantInt::get(index_type, args.size());
        pkg_args = _builder->CreateAlloca(_package_struct_type, args_sz_val);
        const auto ptr = _builder->CreatePointerCast(pkg_args, _c4rt_package_type);
        for (std::size_t i = 0; i < args.size(); ++i) {
            const auto param_ptr = _builder->CreateInBoundsGEP(_package_struct_type, ptr,
                                                               llvm::ConstantInt::get(index_type, i));
            _builder->CreateMemCpy(param_ptr, llvm::Align(8), args[i], llvm::Align(8), C4_PACKAGE_VERSION_1_SIZE);
        }
    }

    llvm::FunctionCallee fn;
    get_rt_function(&fn, "c4rt_datum_evaluate", _c4rt_datum_type, _c4rt_datum_type, ptr_t);
    return _builder->CreateCall(fn, {datum, pkg_args});
}

void
c4rt2c::c4_rt2_emitter::emit_package_init(llvm::Value* ptr) const {
    const auto ptr_t = llvm::PointerType::get(_builder->getContext(), 0);
    const auto void_t = llvm::Type::getVoidTy(_builder->getContext());

    llvm::FunctionCallee fn;
    get_rt_function(&fn, "c4rt_package_init", void_t, ptr_t);
    _builder->CreateCall(fn, {ptr});
}

void
c4rt2c::c4_rt2_emitter::emit_package_init_from_result(llvm::Value* ptr, llvm::Value* datum) const {
    const auto ptr_t = llvm::PointerType::get(_builder->getContext(), 0);
    const auto void_t = llvm::Type::getVoidTy(_builder->getContext());

    llvm::FunctionCallee fn;
    get_rt_function(&fn, "c4rt_package_init_from_result", void_t, ptr_t, _c4rt_datum_type);
    _builder->CreateCall(fn, {ptr, datum});
}

void
c4rt2c::c4_rt2_emitter::emit_package_init_from_function(llvm::Value* pkg,
                                                        llvm::Function* function,
                                                        const std::vector<llvm::Value*>& vector) const {
    const auto ptr_t = llvm::PointerType::get(_builder->getContext(), 0);
    const auto int32_t = llvm::Type::getInt32Ty(_builder->getContext());
    const auto void_t = llvm::Type::getVoidTy(_builder->getContext());
    const auto size_ty = _module.getDataLayout().getIndexType(_module.getContext(), 0);

    const auto args_array = local_package_array_uninit(vector.size());
    const auto array_type = llvm::ArrayType::get(_package_struct_type, vector.size());
    for (std::size_t i = 0; i < vector.size(); ++i) {
        const auto param_ptr = _builder->CreateInBoundsGEP(array_type, args_array,
                                                           {
                                                               llvm::ConstantInt::get(size_ty, 0),
                                                               llvm::ConstantInt::get(size_ty, i),
                                                           });
        _builder->CreateMemCpy(param_ptr, llvm::Align(8), vector[i], llvm::Align(8), C4_PACKAGE_VERSION_1_SIZE);
    }

    llvm::FunctionCallee fn;
    get_rt_function(&fn, "c4rt_package_init_from_function", void_t, ptr_t, ptr_t, ptr_t, int32_t);
    _builder->CreateCall(fn, {
                             pkg, function, args_array,
                             llvm::ConstantInt::get(
                                 int32_t, static_cast<std::uint32_t>(vector.size() * C4_PACKAGE_VERSION_1_SIZE))
                         });
}

void
c4rt2c::c4_rt2_emitter::emit_package_init_from_closure(llvm::Value* pkg,
                                                       llvm::Function* function,
                                                       llvm::Value* ctx,
                                                       const std::span<llvm::Value*> vector) const {
    const auto ptr_t = llvm::PointerType::get(_builder->getContext(), 0);
    const auto int32_t = llvm::Type::getInt32Ty(_builder->getContext());
    const auto void_t = llvm::Type::getVoidTy(_builder->getContext());

    const auto& data_layout = _module.getDataLayout();
    const auto size_ty = data_layout.getIndexType(_module.getContext(), 0);
    const auto ctx_size = data_layout.getTypeAllocSize(ctx->getType());

    // normal args
    const auto args_array = local_package_array_uninit(vector.size());
    const auto array_type = llvm::ArrayType::get(_package_struct_type, vector.size());
    for (std::size_t i = 0; i < vector.size(); ++i) {
        const auto param_ptr = _builder->CreateInBoundsGEP(array_type, args_array,
                                                           llvm::ConstantInt::get(size_ty, i));
        _builder->CreateMemCpy(param_ptr, llvm::Align(8), vector[i], llvm::Align(8), C4_PACKAGE_VERSION_1_SIZE);
    }

    llvm::FunctionCallee fn;
    get_rt_function(&fn, "c4rt_package_init_from_closure", void_t, ptr_t, ptr_t, ptr_t, int32_t, ptr_t, int32_t);
    _builder->CreateCall(fn, {
                             pkg,
                             function,
                             ctx,
                             llvm::ConstantInt::get(int32_t, ctx_size),
                             args_array,
                             llvm::ConstantInt::get(
                                 int32_t, static_cast<std::uint32_t>(vector.size() * C4_PACKAGE_VERSION_1_SIZE)),
                         });
}

llvm::Value*
c4rt2c::c4_rt2_emitter::emit_unpack(llvm::Value* value) const {
    const auto ptr_t = llvm::PointerType::get(_builder->getContext(), 0);

    llvm::FunctionCallee fn;
    get_rt_function(&fn, "c4rt_package_evaluate", _c4rt_datum_type, ptr_t);
    return _builder->CreateCall(fn, {value});
}

llvm::Value*
c4rt2c::c4_rt2_emitter::local_package() const {
    const auto local = _builder->CreateAlloca(_package_struct_type, nullptr, "pkg");
    local->setAlignment(llvm::Align(8));
    set_package_version(local);
    return local;
}

llvm::Value*
c4rt2c::c4_rt2_emitter::local_package_array(const size_t n) const {
    const auto local = local_package_array_uninit(n);
    if (n == 0) return local;

    const auto array_type = llvm::ArrayType::get(_package_struct_type, n);
    for (std::size_t i = 0; i < n; ++i) {
        const auto elem_ptr = _builder->CreateInBoundsGEP(array_type, local,
                                                          llvm::ConstantInt::get(
                                                              llvm::Type::getInt64Ty(_module.getContext()), i));
        set_package_version(elem_ptr);
    }
    return local;
}

llvm::Value*
c4rt2c::c4_rt2_emitter::local_package_array_uninit(const size_t n) const {
    const auto ptr_t = llvm::PointerType::get(_module.getContext(), 0);
    if (n == 0) return llvm::ConstantPointerNull::get(ptr_t);

    const auto local = _builder->CreateAlloca(
        _package_struct_type,
        llvm::ConstantInt::get(_module.getContext(), llvm::APInt(64, n)),
        "args");
    local->setAlignment(llvm::Align(8));
    return local;
}

llvm::Value*
c4rt2c::c4_rt2_emitter::encode_datum(const double d) const {
    const auto datum = c4rt_datum_from_double(d);
    return llvm::ConstantInt::get(_c4rt_datum_type, datum);
}

llvm::Value*
c4rt2c::c4_rt2_emitter::encode_datum_int32(const std::int32_t i) const {
    const auto datum = c4rt_datum_from_int32(i);
    return llvm::ConstantInt::get(_c4rt_datum_type, datum);
}

llvm::Value*
c4rt2c::c4_rt2_emitter::encode_datum_int64(std::int64_t i) const {
    auto& context = _builder->getContext();
    const auto int64_type = llvm::Type::getInt64Ty(context);
    const auto int32_type = llvm::Type::getInt32Ty(context);
    // ReSharper disable once CppDFAMemoryLeak _module takes ownership
    const auto int_global = new llvm::GlobalVariable(_module,
                                                     int64_type,
                                                     true, llvm::GlobalValue::PrivateLinkage,
                                                     llvm::ConstantInt::get(int64_type, i));
    const auto const_int64_datum_type = llvm::ConstantInt::get(int32_type, C4_Integer);

    return emit_datum_from_static_ptr(const_int64_datum_type, int_global);
}

llvm::Value*
c4rt2c::c4_rt2_emitter::encode_datum_string(const std::string_view i) const {
    if (i.size() <= 5) {
        const auto datum = c4rt_datum_from_string_sz(i.data(), i.size());
        return llvm::ConstantInt::get(_c4rt_datum_type, datum);
    }

    auto& context = _builder->getContext();
    const auto int32_type = llvm::Type::getInt32Ty(context);
    const auto ptr_t = llvm::PointerType::get(_builder->getContext(), 0);

    const auto str = _builder->CreateGlobalString(i, "", 0, &_module);
    const auto str_ptr = _builder->CreatePointerCast(str, ptr_t);
    const auto const_int64_datum_type = llvm::ConstantInt::get(int32_type, C4_String);

    return emit_datum_from_static_ptr(const_int64_datum_type, str_ptr);
}

void
c4rt2c::c4_rt2_emitter::set_package_version(llvm::Value* pkg) const {
    const auto version_ptr = _builder->CreateStructGEP(_package_struct_type, pkg, 0, "v");
    _builder->CreateStore(llvm::ConstantInt::get(_module.getContext(), llvm::APInt(16, C4_PACKAGE_VERSION_1)),
                          version_ptr);
}

void
c4rt2c::c4_rt2_emitter::declare_rt_function_impl(const std::string_view name,
                                                 llvm::Type* ret_type,
                                                 std::span<llvm::Type* const> arg_types) const {
    llvm::FunctionType* fn_type = llvm::FunctionType::get(ret_type, {arg_types.data(), arg_types.size()}, false);
    llvm::Function::Create(fn_type, llvm::Function::ExternalLinkage, name, _module);
}

void
c4rt2c::c4_rt2_emitter::get_rt_function_impl(llvm::FunctionCallee* callee, std::string_view name, llvm::Type* ret_type,
                                             std::span<llvm::Type* const> arg_types) const {
    llvm::FunctionType* fn_type = llvm::FunctionType::get(ret_type, {arg_types.data(), arg_types.size()}, false);
    const auto fn = _module.getFunction(name);
    ASSERT(fn, "function {} must be defined in module", name);
    DEBUG_ASSERT(fn->getFunctionType() == fn_type, "function {} must have the same signature", name);

    *callee = llvm::FunctionCallee(fn_type, fn);
}
