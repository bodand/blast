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
#include <memory>
#include <string>
#include <string_view>

#include <c4/diagnostic.hxx>

#include <c4/ast2/ast_context.hxx>

#include <c4/p2/parser.hxx>
#include <c4/p2/source_resolver.hxx>
#include <c4/p2/lex/lexer.hxx>

#include <c4i/gc_registrar.hxx>
#include <c4i/lazy_codegen.hxx>

#include <gc/gc.h>

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
#include <llvm/ExecutionEngine/Orc/CompileUtils.h>
#include <llvm/ExecutionEngine/Orc/Core.h>
#include <llvm/ExecutionEngine/Orc/ExecutionUtils.h>
#include <llvm/ExecutionEngine/Orc/ExecutorProcessControl.h>
#include <llvm/ExecutionEngine/Orc/IRCompileLayer.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h>

#include <sgetopt/opts.hxx>
#include <sgetopt/opts_helper.hxx>

namespace fs = std::filesystem;

namespace {
	#define argdesc(flag, arg, ...) "   " #flag "  "  << std::setw(w) << #arg << "   " #__VA_ARGS__ "\n"

	[[noreturn]] void
	usage() {
		constexpr int w = 5;
		std::cerr << "usage: " << argv0 << " [-hI] <source>\n"
				<< "\n"
				<< "options: \n"
				argdesc(-h, , Print this help and exit 100.)
				argdesc(-I, path, Append path to library search path.)
				//
				;
		exit(100);
	}

	void
	initialize_targets();
}

int
main(int argc, const char* const* argv) try {
	argv0 = argv[0];

	c4::diagnostics_engine diag(stderr);
	c4::p2::source_resolver resolver(diag);
	c4::p2::invalid_file_source internal_src("<c4i>");

	sopts::opts_helper opts{argc, argv};
	while (char opt = opts.step("hI:")) {
		switch (opt) {
		case 'I':
			if (opts.arg() == "-") {
				resolver.reset_path();
				break;
			}
			resolver.push_path(opts.arg());
			break;

		default:
			diag.error(opts.here(), "unrecognized option {}", opt);
			[[fallthrough]];
		case 'h':
			usage();
		}
	}

	if (diag.errored()) return 100;
	opts.shift();
	if (argc < 1) usage();
	opts.shift(1);

	const auto src_path = fs::absolute(argv[0]);
	const auto src = resolver.open_source(src_path);

	initialize_targets();

	const auto target_triple = llvm::sys::getDefaultTargetTriple();
	const auto llvm_triple = llvm::Triple(target_triple);

	std::string target_error;
	const auto target = llvm::TargetRegistry::lookupTarget(llvm_triple, target_error);
	if (!target) {
		diag.error(c4::position::invalid_file_position(internal_src),
		           "cannot create LLVM target triple: {}",
		           target_error);
		return 100;
	}

	llvm::TargetOptions target_options;
	target_options.DataSections = true;
	target_options.FunctionSections = true;

	const auto machine = std::unique_ptr<llvm::TargetMachine>(
		target->createTargetMachine(llvm_triple,
		                            "generic", "",
		                            target_options,
		                            llvm::Reloc::PIC_));

	llvm::orc::LLJITBuilder jit_builder{};
	auto data_layout = machine->createDataLayout();
	jit_builder.setDataLayout(data_layout);
	// TODO: ponder about why multithreading will break everything

	auto jit_ = jit_builder.create();
	if (!jit_) {
		diag.error(c4::position::invalid_file_position(internal_src),
		           "cannot create LLVM JIT: {}",
		           toString(jit_.takeError()));
		return 111;
	}
	auto& jit = *jit_;
	auto& entry_dylib = jit->getMainJITDylib();

	auto self_loader_ = llvm::orc::DynamicLibrarySearchGenerator::
			GetForCurrentProcess(data_layout.getGlobalPrefix());
	if (!self_loader_) {
		diag.error(c4::position::invalid_file_position(internal_src),
		           "cannot create symbol loader to load from process (c4i): {}",
		           toString(self_loader_.takeError()));
		return 111;
	}
	auto& self_loader = self_loader_.get();
	entry_dylib.addGenerator(std::move(self_loader));

	const auto std4_ = resolver.resolve_link_library(
		c4::position::invalid_file_position(internal_src),
		"std4");
	if (!std4_) return 111;

	auto std4_gen_ = llvm::orc::StaticLibraryDefinitionGenerator::Load(
		jit->getObjLinkingLayer(),
		std4_->c_str());
	if (!std4_gen_) {
		diag.error(c4::position::invalid_file_position(internal_src),
		           "cannot build loader for std4: {}",
		           toString(std4_gen_.takeError()))
		    .note("library resolved to {}", std4_->string());
		return 111;
	}
	entry_dylib.addGenerator(std::move(*std4_gen_));

	const auto c4rt3_ = resolver.resolve_link_library(
		c4::position::invalid_file_position(internal_src),
		":libc4rt3.a");
	if (!c4rt3_) return 111;

	auto c4rt3_gen_ = llvm::orc::StaticLibraryDefinitionGenerator::Load(
		jit->getObjLinkingLayer(),
		c4rt3_->c_str());
	if (!c4rt3_gen_) {
		diag.error(c4::position::invalid_file_position(internal_src),
		           "cannot build loader for libc4rt3: {}",
		           toString(c4rt3_gen_.takeError()))
		    .note("library resolved to {}", c4rt3_->string());
		return 111;
	}
	entry_dylib.addGenerator(std::move(*c4rt3_gen_));

	// auto codegen = std::make_unique<c4i::lazy_codegen>(
		// jit->getIRCompileLayer(),
		// diag,
		// resolver
	// );
	// codegen->add_parsed(src_path, );
	// entry_dylib.addGenerator(std::move(codegen));

	auto entry_ = jit->lookup("main");
	if (!entry_) {
		diag.error(c4::position::invalid_file_position(internal_src),
		           "cannot find main function");
		return 111;
	}
	const auto entry = entry_.get();
	const auto entry_ptr = entry.toPtr<int(int, const char* const*)>();

	std::cout << (void*)entry_ptr << "\n";

	return 0;
}
catch (const std::exception& e) {
	std::cerr << argv0 << ": \033[31mfatal:\033[0m " << e.what() << "\n";
	return 111;
}
catch (...) {
	std::cerr << argv0 << ": \033[31mfatal:\033[0m weird error?\n";
	return 111;
}


namespace {
	void
	initialize_targets() {
		llvm::InitializeAllTargetInfos();
		llvm::InitializeAllTargets();
		llvm::InitializeAllTargetMCs();
		llvm::InitializeAllAsmParsers();
		llvm::InitializeAllAsmPrinters();
	}
}
