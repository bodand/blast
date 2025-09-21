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
 * Originally created: 2025-09-09.
 *
 * src/c4i/src/c4i --
 *   
 */

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <filesystem>

#include <c4/ffm.hxx>
#include <c4/ffm_mapper.hxx>

#include <c4/p2/parser.hxx>
#include <c4/p2/lex/lexer.hxx>
#include <c4rt2/datum.h>
#include <c4rt2/datum_type.h>
#include <c4rt2c/ffm_ir_emitter.hxx>

#include <c4rt2c/source_file.hxx>
#include <llvm/ExecutionEngine/RuntimeDyld.h>

#include <lyra/lyra.hpp>

#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/Verifier.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Passes/StandardInstrumentations.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/TargetParser/Host.h>

#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/ExecutionEngine/Orc/Core.h>
#include <llvm/ExecutionEngine/Orc/ExecutionUtils.h>
#include <llvm/ExecutionEngine/Orc/CompileUtils.h>
#include <llvm/ExecutionEngine/Orc/ExecutorProcessControl.h>
#include <llvm/ExecutionEngine/Orc/IRCompileLayer.h>
#include <llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h>

struct jit {
    jit(std::unique_ptr<llvm::orc::ExecutionSession> execution_session,
        llvm::orc::JITTargetMachineBuilder jit_builder,
        llvm::DataLayout data_layout)
        : _execution_session{std::move(execution_session)}
        , _data_layout{data_layout}
        , _mangle{*_execution_session, _data_layout}
        , _linker{
            *_execution_session,
            [] {
                return std::make_unique<llvm::SectionMemoryManager>();
            }
        }
        , _compiler{
            *_execution_session, _linker, std::make_unique<llvm::orc::ConcurrentIRCompiler>(std::move(jit_builder))
        }
        , _main_jit_dylib{_execution_session->createBareJITDylib("<main>")} {
        _main_jit_dylib.addGenerator(
            llvm::cantFail(llvm::orc::DynamicLibrarySearchGenerator::GetForCurrentProcess(_data_layout.getGlobalPrefix())));
        _main_jit_dylib.addGenerator(
            llvm::cantFail(llvm::orc::DynamicLibrarySearchGenerator::Load("gc.dll", _data_layout.getGlobalPrefix())));

        if (jit_builder.getTargetTriple().isOSBinFormatCOFF()) {
            _linker.setOverrideObjectFlagsWithResponsibilityFlags(true);
            _linker.setAutoClaimResponsibilityForObjectSymbols(true);
        }
    }

    ~jit() {
        if (auto err = _execution_session->endSession())
            _execution_session->reportError(std::move(err));
    }

    static llvm::Expected<std::unique_ptr<jit>>
    create() {
        auto process_control = llvm::orc::SelfExecutorProcessControl::Create();
        if (!process_control) return process_control.takeError();

        auto execution_session = std::make_unique<llvm::orc::ExecutionSession>(std::move(*process_control));
        auto jit_builder = llvm::orc::JITTargetMachineBuilder::detectHost();
        if (!jit_builder) return jit_builder.takeError();

        auto data_layout = jit_builder->getDefaultDataLayoutForTarget();
        if (!data_layout) return data_layout.takeError();

        return std::make_unique<jit>(std::move(execution_session), std::move(*jit_builder), std::move(*data_layout));
    }

    llvm::Error
    addModule(llvm::orc::ThreadSafeModule thread_safe_module) {
        return _compiler.add(_main_jit_dylib, std::move(thread_safe_module));
    }

    llvm::Expected<llvm::orc::ExecutorSymbolDef>
    lookup(const std::string_view name) {
        return _execution_session->lookup({&_main_jit_dylib}, _mangle(name));
    }

    [[nodiscard]] const llvm::DataLayout&
    data_layout() const { return _data_layout; }

private:
    std::unique_ptr<llvm::orc::ExecutionSession> _execution_session;
    llvm::DataLayout _data_layout;
    llvm::orc::MangleAndInterner _mangle;
    llvm::orc::RTDyldObjectLinkingLayer _linker;
    llvm::orc::IRCompileLayer _compiler;
    llvm::orc::JITDylib& _main_jit_dylib;
};

void
initialize_targets();

int
main(int argc, char** argv) {
    std::filesystem::path src_path;
    bool show_help = false;

    const auto cli = lyra::cli()
                     | lyra::help(show_help).description(
                         "Run a C4 script using LLVM's ORC JIT.")(
                         "Do not compile, print help and exit.")
                     | lyra::arg(src_path, "source")("The C4 source file to JIT compile.").required()
            //
            ;

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

    src_path = absolute(src_path);
    c4::ast2::ast_context ast_context;
    c4::p2::lexer lexer(src_path.string(), src.begin(), src.end());
    c4::diagnostics_engine diagnostics_engine{stderr, false};
    c4::p2::parser parser(ast_context, diagnostics_engine, std::move(lexer));

    parser.declare_binop("+", 4, false);
    parser.declare_binop("-", 4, false);
    parser.declare_binop("*", 5, false);
    parser.declare_binop("/", 5, false);
    parser.declare_binop("^", 5, true);
    parser.declare_binop("==", 3, true);

    parser.declare_binop("<<", 6, false);
    parser.declare_binop(">>", 5, true);

    parser.declare_uniop("~");
    parser.declare_uniop("+");
    parser.declare_uniop("!");

    parser.declare_symbol("print", 1, nullptr);
    parser.declare_symbol("println", 1, nullptr);
    parser.declare_symbol("if", 3, nullptr);
    parser.declare_symbol("int", 1, nullptr);
    parser.declare_symbol("str_empty", 1, nullptr);
    parser.declare_symbol("blk_empty", 1, nullptr);
    parser.declare_symbol("nil_block", 0, nullptr);
    parser.declare_symbol("cat", 2, nullptr);
    parser.declare_symbol("readln", 0, nullptr);

    try {
        initialize_targets();
        auto jit = jit::create();
        if (!jit) {
            std::cerr << "fatal: cannot create JIT instance\n";
            return 1;
        }
        auto* jit_ptr = jit->get();

        const auto script = parser.parse_script();
        if (diagnostics_engine.errored())
            return 1;

        c4::ffm::ffm_context ffm_context;
        c4::ffm_mapper mapper(diagnostics_engine, ffm_context);
        for (const auto& expression : script) expression->accept(mapper);
        mapper.finalize_block_body();

        if (diagnostics_engine.errored())
            return 1;

        const auto roots = mapper.roots();

        auto context = std::make_unique<llvm::LLVMContext>();

        const auto module_id = src_path.string();
        auto module = std::make_unique<llvm::Module>(module_id, *context);
        module->setSourceFileName(module_id);
        module->setDataLayout(jit_ptr->data_layout());

        const auto target_triple = llvm::sys::getDefaultTargetTriple();
        std::string target_error;
        const auto target = llvm::TargetRegistry::lookupTarget(target_triple, target_error);
        if (!target) {
            std::cerr << "fatal: " << target_error << "\n";
            return 2;
        }
        module->setTargetTriple(target_triple);

        llvm::FunctionPassManager fn_pm;
        llvm::LoopAnalysisManager loop_am;
        llvm::FunctionAnalysisManager fn_am;
        llvm::ModuleAnalysisManager mod_am;
        llvm::CGSCCAnalysisManager cgscc_am;
        llvm::PassInstrumentationCallbacks pass_ic;
        llvm::StandardInstrumentations si(*context, false);
        si.registerCallbacks(pass_ic, &mod_am);

        llvm::PassBuilder pass_builder;
        pass_builder.registerModuleAnalyses(mod_am);
        pass_builder.registerFunctionAnalyses(fn_am);
        pass_builder.crossRegisterProxies(loop_am, fn_am, cgscc_am, mod_am);

        llvm::IRBuilder<> builder(*context);
        c4c::ffm_ir_emitter ir(*context, *module, builder, fn_pm, fn_am);
        for (const auto& ffm_entry : roots) {
            ffm_entry->accept(ir);
        }

        llvm::orc::ThreadSafeModule ts_module(std::move(module), std::move(context));
        if (auto err = jit_ptr->addModule(std::move(ts_module));
            err) {
            std::cerr << "fatal: cannot add module to jit\n";
            return 1;
        }

        auto main = jit_ptr->lookup("_C@main");
        if (!main) {
            std::ignore = main.takeError();
            std::cerr << "fatal: cannot find main function\n";
            return 1;
        }

        const auto entry = main->getAddress().toPtr<c4_datum_t(*)()>();
        const auto res = entry();

        return c4rt_datum_coerce_int32(res);
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
