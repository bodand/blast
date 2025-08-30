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
 * src/c4c/src/global_constant_emitter --
 *   
 */

#include <c4/ast2/integer_literal.hxx>
#include <c4/ast2/float_literal.hxx>
#include <c4/ast2/string_literal.hxx>

#include <c4c/global_constant_emitter.hxx>
#include <c4c/llvm_value_attribute.hxx>

#include <c4rt2/datum.h>

#include <libassert/assert.hpp>

#include <llvm/IR/Constants.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

c4c::global_constant_emitter::global_constant_emitter(llvm::LLVMContext& context,
                                                      llvm::Module& module,
                                                      llvm::IRBuilder<>& builder)
    : context(context)
    , module(module)
    , builder(builder) { }

llvm::Value*
c4c::global_constant_emitter::get_loaded_global(const llvm::Twine& name) {
    ASSERT(value, "global_constant_emitter needs to visit the value before it can generate the load to it");
    value->setName(name.concat(_value_type_suffix));
    return builder.CreateLoad(llvm::Type::getInt64Ty(context), value);
}

void
c4c::global_constant_emitter::do_visit(const c4::ast2::float_literal& obj) {
    _value_type_suffix = "_fl";
    create_global(c4rt_datum_from_double(obj.value()));
}

void
c4c::global_constant_emitter::do_visit(const c4::ast2::integer_literal& obj) {
    ASSERT(obj.value() < std::numeric_limits<std::int32_t>::max());
    _value_type_suffix = "_il";
    create_global(c4rt_datum_from_int32(static_cast<std::int32_t>(obj.value())));
    obj.emplace_attribute<llvm_value_attribute>("value", value);
}

void
c4c::global_constant_emitter::do_visit(const c4::ast2::string_literal& obj) {
    ASSERT(obj.value().size() < c4::ast2::string_literal::short_string_limit);
    _value_type_suffix = "_ssl";
    create_global(c4rt_datum_from_string_sz(obj.value().data(), obj.value().size()));
}

void
c4c::global_constant_emitter::create_global(const c4_datum_t datum) {
    const auto val = new llvm::GlobalVariable(llvm::Type::getInt64Ty(context), true,
                                              llvm::GlobalValue::PrivateLinkage,
                                              llvm::ConstantInt::get(llvm::Type::getInt64Ty(context), datum));
    value = val;
    module.insertGlobalVariable(val);
}
