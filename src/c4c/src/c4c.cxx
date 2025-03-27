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
 * src/c4c/src/c4c --
 *   
 */

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <fstream>
#include <ranges>

#include <c4/ast_dumper.hxx>

#include <c4/p2/parser.hxx>
#include <c4/p2/lex/lexer.hxx>
#include <c4c/c4_runtime_emitter.hxx>

#include <c4c/source_file.hxx>
#include <c4rt/datum.h>
#include <libassert/assert.hpp>

#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/STLExtras.h>
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

#include <lyra/lyra.hpp>

#ifdef _WIN32
#  include <windows.h>
#endif

using namespace std::literals;

struct ostream_deleter {
    void
    operator()(const std::ostream* os) const {
        if (os == &std::cout) return;
        delete os;
    }
};

using outstream_ptr = std::unique_ptr<std::ostream, ostream_deleter>;

outstream_ptr
open_outstream(const std::filesystem::path& path) {
    if (path == "-") return outstream_ptr(&std::cout);

    auto ptr = outstream_ptr(new std::ofstream(path));
    if (!*ptr) throw std::runtime_error("could not open output file: " + path.string());

    return ptr;
}

void
dump_ast(const std::span<const c4::ast2::expression> script,
         std::ostream& out) {
    c4::ast_dumper dumper(out);
    for (const auto& expression: script) {
        expression.accept(dumper);
        out << "\n";
    }
}

void
initialize_targets();

struct fn_body_emitter final : c4::ast2::visitor<c4::ast2::block,
                                                 c4::ast2::block_args,
                                                 c4::ast2::fn_call
                                                 , c4::ast2::binary_op_call
                                                 , c4::ast2::unary_op_call
        > {
    fn_body_emitter(llvm::Function* function,
                    llvm::LLVMContext& context,
                    llvm::Module& module,
                    llvm::IRBuilder<>& builder)
        : function{function}
        , context{context}
        , module{module}
        , builder{builder} { }

    void
    do_visit(const c4::ast2::fn_call& obj) override {
        const auto fn_sym = obj.sym();
        if (const auto arg = _named_arguments.find(fn_sym.name());
            arg != _named_arguments.end()) {
            ASSERT(obj.args().empty(), "parameter cannot be called with parameters...");
            last_val = arg->second;
            return;
        }

        std::vector<llvm::Value*> call_args;
        call_args.reserve(obj.args().size());
        for (const auto& arg: obj.args()) {
            arg.accept_skip_self(*this);
            call_args.push_back(last_val);
        }

        const auto fn = module.getFunction(fn_sym.mangle());
        ASSERT(fn);

        last_val = builder.CreateCall(fn, call_args);
    }

    void
    do_visit(const c4::ast2::binary_op_call& obj) override {
        const auto sym = obj.op();

        const auto fn = module.getFunction(sym.mangle());
        ASSERT(fn);

        obj.left().accept_skip_self(*this);
        const auto lhs = last_val;
        obj.right().accept_skip_self(*this);
        const auto rhs = last_val;
        last_val = builder.CreateCall(fn, {lhs, rhs});
    }

    void
    do_visit(const c4::ast2::unary_op_call& obj) override {
        const auto sym = obj.op();

        const auto fn = module.getFunction(sym.mangle());
        ASSERT(fn);

        obj.operand().accept_skip_self(*this);
        last_val = builder.CreateCall(fn, {last_val});
    }

    void
    do_visit(const c4::ast2::block& obj) override {
        if (obj.args()) obj.args()->accept(*this);

        const auto mem = builder.GetInsertBlock();

        const auto bb = llvm::BasicBlock::Create(context, "", function);
        builder.SetInsertPoint(bb);
        for (const auto& expr: obj.expressions()) {
            expr.accept_skip_self(*this);
        }
        if (last_val) {
            builder.CreateRet(last_val);
        }
        else {
            builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt64Ty(context),
                                                     gC4_Empty_Block));
        }

        builder.SetInsertPoint(mem);
    }

    void
    do_visit(const c4::ast2::block_args& obj) override {
        DEBUG_ASSERT(function);

        for (const auto& [ast_arg, fn_arg]: std::views::zip(obj.args(), function->args())) {
            fn_arg.setName(ast_arg.name());
            _named_arguments[ast_arg.name()] = &fn_arg;
        }
    }

    llvm::Value* last_val{};
    std::unordered_map<std::string_view, llvm::Argument*> _named_arguments;
    llvm::Function* function;
    llvm::LLVMContext& context;
    llvm::Module& module;
    llvm::IRBuilder<>& builder;
};

struct top_level_function_emitter final : c4::ast2::visitor<c4::ast2::let_expression> {
    top_level_function_emitter(c4c::c4_runtime_emitter& rt_emitter,
                               llvm::LLVMContext& context,
                               llvm::Module& module,
                               llvm::IRBuilder<>& builder)
        : rt_emitter{rt_emitter}
        , context{context}
        , module{module}
        , builder{builder} { }

    void
    do_visit(const c4::ast2::let_expression& obj) override {
        const auto sym_name = obj.mangled_name();
        const auto arity = obj.symbol_arity();

        const auto fn_type = rt_emitter.get_c4_funtype(arity);
        auto fn = module.getFunction(sym_name);
        if (!fn)
            fn = llvm::Function::Create(fn_type, llvm::GlobalValue::ExternalLinkage,
                                        sym_name,
                                        module);

        fn_body_emitter body_emitter(fn, context, module, builder);
        obj.value().accept_skip_self(body_emitter);

        // const auto memory = builder.GetInsertBlock();

        // if (fn->empty()) llvm::BasicBlock::Create(context, "", fn);
        // auto& fn_main_block = fn->back();
        // builder.SetInsertPoint(&fn_main_block);
        // const auto ret = c4rt_datum_from_int32(0);
        // builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt64Ty(context), ret));

        // builder.SetInsertPoint(memory);
    }

    c4c::c4_runtime_emitter& rt_emitter;
    llvm::LLVMContext& context;
    llvm::Module& module;
    llvm::IRBuilder<>& builder;
};

struct top_level_main_emitter final : c4::ast2::visitor<c4::ast2::fn_call,
                                                        c4::ast2::binary_op_call,
                                                        c4::ast2::unary_op_call,
                                                        c4::ast2::integer_literal> {
    top_level_main_emitter(c4c::c4_runtime_emitter& rt_emitter,
                           llvm::LLVMContext& context,
                           llvm::Module& module,
                           llvm::IRBuilder<>& builder)
        : val{nullptr}
        , rt_emitter{rt_emitter}
        , context{context}
        , module{module}
        , builder{builder} { }

    void
    do_visit(const c4::ast2::fn_call& obj) override {
        const auto fn_sym = obj.sym();

        const auto sym_name = fn_sym.mangle();
        const auto fn = module.getFunction(sym_name);
        ASSERT(fn);

        for (auto arg: obj.args()) arg.accept_skip_self(*this);
        val = builder.CreateCall(fn, {val});
    }

    void
    do_visit(const c4::ast2::binary_op_call& obj) override { }

    void
    do_visit(const c4::ast2::unary_op_call& obj) override { }

    void
    do_visit(const c4::ast2::integer_literal& obj) override {
        if (const auto val = obj.value();
            val < std::numeric_limits<int32_t>::max()) {
            const auto ret = c4rt_datum_from_int32(obj.value());
            this->val = llvm::ConstantInt::get(llvm::Type::getInt64Ty(context), ret);
        }
        else {
            const auto ret = c4rt_datum_from_int64(obj.value());
            this->val = llvm::ConstantInt::get(llvm::Type::getInt64Ty(context), ret);
        }
    }

    llvm::Value* val;
    c4c::c4_runtime_emitter& rt_emitter;
    llvm::LLVMContext& context;
    llvm::Module& module;
    llvm::IRBuilder<>& builder;
};

int
main(int argc, const char** argv) {
    std::filesystem::path out_path;
    std::filesystem::path src_path;
    std::string target_arch;
    std::string dump_type;
    bool show_help = false;

    const auto cli = lyra::cli()
                     | lyra::help(show_help).description(
                         "Compile a C4 script into object an object file.")(
                         "Do not compile, print help and exit.")
                     | lyra::opt(out_path, "output")["-o"]["--output"](
                         "The name of the output file. When -d is set, STDOUT if `-'.")
                     | lyra::opt(dump_type, "dump")["-d"]["--dump"](
                         "Do not compile, dump code instead. [AST, IR, ASM]").choices("AST", "IR", "ASM")
                     | lyra::arg(src_path, "source")(
                         "The C4 source file to compile.").required()
                     | lyra::opt(target_arch, "target arch triplet")["-T"]["--target"](
                         "The target triplet to produce the binary for.");

    if (const auto result = cli.parse({argc, argv});
        !result) {
        std::cerr << "fatal: " << result.message() << "\n";
        std::cerr << cli << std::endl;
        return 1;
    }

    if (show_help) {
        std::cout << cli << std::endl;
        return 1;
    }

    c4c::source_file src(src_path);
    if (out_path.empty()) {
        out_path = src_path;
        out_path.replace_extension(".o");
    }

    src_path = absolute(src_path);
    c4::p2::lexer lexer(src_path.string(), src.begin(), src.end());
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

    try {
        const auto script = parser.parse_script();
        if (!parser.valid()) return 1;

        if (dump_type == "AST") {
            auto outstrm = open_outstream(out_path);
            dump_ast(script, *outstrm);
            return 0;
        }

        initialize_targets();

        llvm::LLVMContext context;
        llvm::Module module(src_path.string(), context);
        llvm::IRBuilder<> builder(context);

        const auto target_triple = target_arch.empty()
                                   ? llvm::sys::getDefaultTargetTriple()
                                   : target_arch;
        std::string target_error;
        const auto target = llvm::TargetRegistry::lookupTarget(target_triple, target_error);
        if (!target) {
            std::cerr << "fatal: " << target_error << "\n";
            return 2;
        }

        const auto machine = target->createTargetMachine(target_triple, "generic", "", {}, llvm::Reloc::PIC_);
        module.setDataLayout(machine->createDataLayout());
        module.setTargetTriple(target_triple);

        auto entry_type = llvm::FunctionType::get(llvm::Type::getInt32Ty(context), false);
        auto crt = llvm::Function::Create(entry_type, llvm::Function::ExternalLinkage,
                                          "mainCRTStartup",
                                          module);
        auto crt_bb = llvm::BasicBlock::Create(context, "", crt);
        builder.SetInsertPoint(crt_bb);

        c4c::c4_runtime_emitter rt_emitter(context, module);
        for (const auto& sym: parser.promised_symbols()) {
            const auto fn_type = rt_emitter.get_c4_funtype(sym.arity);
            llvm::Function::Create(fn_type, llvm::Function::ExternalLinkage, sym.mangle(), module);
        }

        top_level_function_emitter tl_fn_emitter(rt_emitter, context, module, builder);
        top_level_main_emitter main_emitter(rt_emitter, context, module, builder);

        for (const auto& expression: script) {
            expression.accept_skip_self(tl_fn_emitter);
            expression.accept_skip_self(main_emitter);
        }

        const auto main_ret_call = rt_emitter.emit_rt_call(c4c::c4rt_symbol::DatumCoerceInt32, builder,
                                                           {main_emitter.val});
        builder.CreateRet(main_ret_call);

        if (dump_type == "IR") {
            std::string dump;
            llvm::raw_string_ostream os(dump);
            module.print(os, nullptr);
            *open_outstream(out_path) << dump;
            return 0;
        }

        if (out_path == "-" && dump_type != "ASM") {
            std::cerr << "fatal: cowardly refusing to dump binary data to STDOUT\n";
            return 1;
        }

        std::error_code ec;
        llvm::raw_fd_ostream fout(out_path.string(), ec, llvm::sys::fs::OF_None);
        if (ec) {
            std::cerr << "fatal: could not open file `" << out_path << "'\n";
            return 1;
        }

        auto out_type = llvm::CodeGenFileType::ObjectFile;
        if (dump_type == "ASM") out_type = llvm::CodeGenFileType::AssemblyFile;

        llvm::legacy::PassManager pass_mgr;
        machine->addPassesToEmitFile(pass_mgr,
                                     fout,
                                     nullptr,
                                     out_type);
        pass_mgr.run(module);
        fout.flush();
    }
    catch (const std::exception& e) {
        std::cerr << "fatal: " << e.what() << "\n";
        return 1;
    }
}

void
initialize_targets() {
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
}
