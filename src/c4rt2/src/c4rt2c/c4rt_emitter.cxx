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

c4rt2c::c4_rt2_emitter::
c4_rt2_emitter(llvm::Module& module, llvm::IRBuilder<>* builder)
    : _c4rt_datum_type{llvm::Type::getInt64Ty(builder->getContext())}
    , _c4rt_package_type{llvm::PointerType::get(builder->getContext(), 0)}
    , _builder{builder}
    , _module{module} {
    const auto int64_t = llvm::Type::getInt64Ty(_builder->getContext());
    const auto int16_t = llvm::Type::getInt16Ty(_builder->getContext());
    _package_struct_type = llvm::StructType::get(_module.getContext(), {int64_t, int64_t, int64_t}, true);

    const auto type_t = llvm::Type::getInt32Ty(_builder->getContext());
    const auto ptr_t = llvm::PointerType::get(_builder->getContext(), 0);
    const auto void_t = llvm::Type::getVoidTy(_builder->getContext());

    declare_rt_function("c4rt_datum_from_static_ptr", _c4rt_datum_type, type_t, ptr_t);
    declare_rt_function("c4rt_datum_evaluate", _c4rt_datum_type, _c4rt_datum_type);
    declare_rt_function("c4rt_package_init", void_t, ptr_t);
    declare_rt_function("c4rt_package_set_from_function", void_t, ptr_t, ptr_t, ptr_t, int16_t);
    declare_rt_function("c4rt_package_set_from_result", void_t, ptr_t, _c4rt_datum_type);
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
c4rt2c::c4_rt2_emitter::emit_datum_evaluate(llvm::Value* datum) const {
    llvm::FunctionCallee fn;
    get_rt_function(&fn, "c4rt_datum_evaluate", _c4rt_datum_type, _c4rt_datum_type);
    return _builder->CreateCall(fn, {datum});
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
c4rt2c::c4_rt2_emitter::emit_package_set_from_result(llvm::Value* ptr, llvm::Value* datum) const {
    const auto ptr_t = llvm::PointerType::get(_builder->getContext(), 0);
    const auto void_t = llvm::Type::getVoidTy(_builder->getContext());

    llvm::FunctionCallee fn;
    get_rt_function(&fn, "c4rt_package_set_from_result", void_t, ptr_t, _c4rt_datum_type);
    _builder->CreateCall(fn, {ptr, datum});
}

void
c4rt2c::c4_rt2_emitter::emit_package_set_from_function(llvm::Value* pkg,
                                                       llvm::Function* function,
                                                       const std::vector<llvm::Value*>& vector) const {
    const auto int16_t = llvm::Type::getInt16Ty(_builder->getContext());
    const auto ptr_t = llvm::PointerType::get(_builder->getContext(), 0);
    const auto void_t = llvm::Type::getVoidTy(_builder->getContext());

    const auto index_type = _module.getDataLayout().getIndexType(_module.getContext(), 0);
    const auto mem_type = llvm::ArrayType::get(_package_struct_type, vector.size());
    const auto mem = _builder->CreateAlloca(_package_struct_type,
                                            llvm::ConstantInt::get(index_type, vector.size()),
                                            "pkg_params");
    const auto ptr = _builder->CreatePointerCast(mem, _c4rt_package_type);
    for (std::size_t i = 0; i < vector.size(); ++i) {
        const auto param_ptr = _builder->CreateInBoundsGEP(mem_type, ptr,llvm::ConstantInt::get(index_type, i));
        _builder->CreateStore(vector[i], param_ptr);
    }

    const auto arity = static_cast<uint16_t>(vector.size());
    const auto arity_val = llvm::ConstantInt::get(int16_t, arity);

    llvm::FunctionCallee fn;
    get_rt_function(&fn, "c4rt_package_set_from_function", void_t, ptr_t, ptr_t, ptr_t, int16_t);
    _builder->CreateCall(fn, {pkg, function, ptr, arity_val});
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
    const auto ptr = _builder->CreatePointerCast(local, _c4rt_package_type);
    emit_package_init(ptr);
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
