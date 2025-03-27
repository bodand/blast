/* demo project
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
 * Originally created: 2025-02-02.
 *
 * src/blast --
 *   
 */

#include <iomanip>
#include <iostream>

#include <fmt/format.h>
#include <fmt/std.h>

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif

#include <c4/ast2/visitor/visitor.hxx>
#include <c4/p2/parser.hxx>
#include <c4/p2/lex/lexer.hxx>

// #define C4RT_STATIC
#include <memory>
#include <ranges>
#include <c4/ast_dumper.hxx>
#include <c4rt/datum.h>

#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/ExecutionEngine/Orc/Core.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/IR/LegacyPassManager.h>

using namespace std::literals;

struct value_visitor final : c4::ast2::visitor<
            c4::ast2::string_literal,
            c4::ast2::float_literal,
            c4::ast2::integer_literal> {
    explicit
    value_visitor(llvm::IRBuilderBase& builder,
                  llvm::Module* module,
                  llvm::LLVMContext* context)
        : value{nullptr}
        , context{context}
        , module{module}
        , _builder{builder} {
        const auto str_maker_type = llvm::FunctionType::get(llvm::Type::getInt64Ty(*context),
                                                            {llvm::Type::getInt8Ty(*context)->getPointerTo()},
                                                            false);
        str_maker = llvm::Function::Create(str_maker_type,
                                           llvm::GlobalValue::ExternalLinkage,
                                           "c4rt_datum_from_string", *module);
        const auto int64_maker_type = llvm::FunctionType::get(llvm::Type::getInt64Ty(*context),
                                                              {llvm::Type::getInt64Ty(*context)},
                                                              false);
        int64_maker = llvm::Function::Create(int64_maker_type,
                                             llvm::GlobalValue::ExternalLinkage,
                                             "c4rt_datum_from_int64", *module);

        const auto cleanup_type = llvm::FunctionType::get(llvm::Type::getVoidTy(*context),
                                                          {llvm::Type::getInt64Ty(*context)},
                                                          false);
        cleanup = llvm::Function::Create(cleanup_type, llvm::GlobalValue::ExternalLinkage, "c4rt_datum_free", *module);
    }

    void
    do_visit(const c4::ast2::string_literal& obj) override {
        const auto val = obj.value();
        const auto str = _builder.CreateGlobalStringPtr(val, "", 0, module);
        value = _builder.CreateCall(str_maker, {str});
        _need_cleanup = true;
    }

    void
    do_visit(const c4::ast2::float_literal& obj) override {
        const auto val = obj.value();
        value = llvm::ConstantInt::get(*context, llvm::APInt(64, c4rt_datum_from_double(val)));
    }

    void
    do_visit(const c4::ast2::integer_literal& obj) override {
        if (const auto val = obj.value();
            val > std::numeric_limits<int32_t>::max()) {
            const auto int64 = llvm::ConstantInt::get(*context, llvm::APInt(64, val));
            value = _builder.CreateCall(int64_maker, {int64});
            _need_cleanup = true;
        }
        else {
            value = llvm::ConstantInt::get(*context, llvm::APInt(64, c4rt_datum_from_int32(val)));
        }
    }

    void
    do_cleanup() {
        if (!_need_cleanup) return;
        _builder.CreateCall(cleanup, {value});
    }

    llvm::Value* value;
    llvm::LLVMContext* context;
    llvm::Module* module;
    llvm::Function* str_maker;
    llvm::Function* int64_maker;
    llvm::Function* cleanup;
    bool _need_cleanup{};
    llvm::IRBuilderBase& _builder;
};

int
main() {
#ifdef _WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif

    constexpr auto buf = R"__(
# asdasd
let (-?)/2 left 2 { |a b| b - a }
let (^) { |x| print x }
let else/1 \|x| x

let x readln

if str_empty x {
    println "Nem adtál meg szöveget"
}
else {
    println cat "A szöveged: " x
}

let default/1 { |x| { |val| x } }
let case/3 { |x code next|
    { |val|
        if x == val code
        else &(next)/1 val
    }
}
let switch/2 { |val case|
    &(case)/1 val
}
let y readln
println
    cat "Párja: "
        switch y
            case 0 10
            case 1 11
            default (add -1 y)

let printn/1 { |n|
    let printn_impl/1 { |n|
        print cat n " "
        if n == 0 {}
            printn_impl add -1 n
    }
    printn_impl n
    println ""
}
printn 24

let xsd
    if readln == 0
        let asd "a"
        let bsd "b"

println &(xsd)/0

let különben/1 \|x| x
0
)__"sv;
    c4::p2::lexer lexer("<string>", buf.data(), buf.data() + buf.size());
    c4::p2::parser parser(std::move(lexer));

    parser.declare_binop("+", 4, false);
    parser.declare_binop("-", 4, false);
    parser.declare_binop("*", 5, false);
    parser.declare_binop("/", 5, false);
    parser.declare_binop("^", 5, true);
    parser.declare_binop("==", 3, true);

    parser.declare_binop("<<", 6, false);
    parser.declare_binop(">>", 5, true);

    parser.declare_uniop("~");

    parser.declare_symbol("print", 1);
    parser.declare_symbol("println", 1);
    parser.declare_symbol("add", 2);
    parser.declare_symbol("if", 3);
    parser.declare_symbol("str_empty", 1);
    parser.declare_symbol("cat", 2);
    parser.declare_symbol("readln", 0);

    llvm::LLVMContext context;
    llvm::Module module("c4-jit", context);
    llvm::IRBuilder<> builder(context);

    auto coerce_to_int_type = llvm::FunctionType::get(llvm::Type::getInt32Ty(context),
                                                      {llvm::Type::getInt64Ty(context)},
                                                      false);
    auto coerce_to_int = llvm::Function::Create(coerce_to_int_type,
                                                llvm::Function::ExternalLinkage,
                                                "c4rt_datum_coerce_int32_t",
                                                module);

    auto entry_type = llvm::FunctionType::get(llvm::Type::getInt32Ty(context), false);
    auto entry = llvm::Function::Create(entry_type,
                                        llvm::Function::PrivateLinkage,
                                        "_c4_entry_point",
                                        module);
    auto entry_block = llvm::BasicBlock::Create(context, "", entry);

    auto crt = llvm::Function::Create(entry_type, llvm::Function::ExternalLinkage,
                                      "mainCRTStartup",
                                      module);
    auto crt_bb = llvm::BasicBlock::Create(context, "", crt);
    builder.SetInsertPoint(crt_bb);
    auto main_ret = builder.CreateCall(entry);
    builder.CreateRet(main_ret);

    builder.SetInsertPoint(entry_block);

    LLVMInitializeX86Target();
    LLVMInitializeX86TargetInfo();
    LLVMInitializeX86TargetMC();
    LLVMInitializeX86AsmPrinter();

    LLVMInitializeAArch64Target();
    LLVMInitializeAArch64TargetInfo();
    LLVMInitializeAArch64TargetMC();
    LLVMInitializeAArch64AsmPrinter();

    LLVMInitializeRISCVTarget();
    LLVMInitializeRISCVTargetInfo();
    LLVMInitializeRISCVTargetInfo();
    LLVMInitializeRISCVTargetMC();
    LLVMInitializeRISCVAsmPrinter();

    LLVMInitializeSparcTarget();
    LLVMInitializeSparcTargetInfo();
    LLVMInitializeSparcTargetMC();
    LLVMInitializeSparcAsmPrinter();

    LLVMInitializeSystemZTarget();
    LLVMInitializeSystemZTargetInfo();
    LLVMInitializeSystemZTargetMC();
    LLVMInitializeSystemZAsmPrinter();

    try {
        const auto exp = parser.parse_script();
        c4::ast_dumper dumper(std::cout);
        for (const auto& expr : exp) {
            expr.accept(dumper);
            std::cout << std::endl;
        }
        if (!parser.valid()) return 1;

        const auto& last_val = exp.back();
        value_visitor visitor(builder, &module, &context);
        last_val.accept_skip_self(visitor);

        const auto coerced = builder.CreateCall(coerce_to_int, {visitor.value});
        visitor.do_cleanup();
        builder.CreateRet(coerced);
        verifyFunction(*entry);

        std::string err;
        const auto target_triple = llvm::sys::getDefaultTargetTriple();
        const auto target = llvm::TargetRegistry::lookupTarget(target_triple, err);
        if (!target) {
            llvm::errs() << "error: " << err;
            return 1;
        }

        const auto machine = target->createTargetMachine(target_triple, "generic", "", {}, llvm::Reloc::PIC_);

        module.setDataLayout(machine->createDataLayout());
        module.setTargetTriple(target_triple);

        module.dump();

        const auto fname = "c4.o";
        std::error_code ec;
        llvm::raw_fd_ostream fout(fname, ec, llvm::sys::fs::OF_None);
        if (ec) {
            llvm::errs() << "error: could not open file '" << fname << "'\n";
            return 1;
        }

        llvm::legacy::PassManager pass_mgr;
        machine->addPassesToEmitFile(pass_mgr, fout, nullptr, llvm::CodeGenFileType::ObjectFile);
        pass_mgr.run(module);
        fout.flush();
    }
    catch (c4::p2::bad_token_error const&) {
        return 1;
    }

    // print "Szöveg:"
    // let x/0 &readln/0
    //
    // if str_empty x {
    //     println "Nem adtál meg szöveget"
    // }
    // else
    //     println cat "A szöveged: " x
    //

    //
    // let printn/1 { |n|
    //     let printn_impl/1 { |n|
    //         if eq 0 n {} {
    //             printn_impl add -1 n
    //             print cat n " "
    //         }
    //     }
    //     printn_impl n
    //     println ""
    // }
    // printn 35
    //
    // print "Szöveg:"
    // let x/0 &{
    //     let x/0 readln
    //     x
    // }/0
    //
    // if str_empty x {
    //     println "Nem adtál meg szöveget"
    // }
    // else
    //     println cat "A szöveged: " x


    // if {} { print "yes" } { print "no" }
    //
    // let else/1 \|x| x
    // let then/1 \|x| x
    //
    // if {} then { print "yes" } else { print "no" }

    // let a/0 0
    // let inc/1 { |x|
    //     print "asd"
    //     add 1 41
    // }
    //
    // print &1/0
    //
    // print
    //     if a "true0" "false0"
    // print
    //     if a \"true0" \"false0"
    // print
    //     if a then "true1" else "false1"
    // print
    //     if a "true2" else "false2"
    // print
    //     if a { "true3" } { "false3" }
    // print
    //     if a { "true4" } else { "false4" }
    // print
    //     if a
    //     then {
    //         "true5"
    //     }
    //     else {
    //         "false5"
    //     }
}
