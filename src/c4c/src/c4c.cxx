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
#include <utility>

#include <c4/ast_dumper.hxx>

#include <c4/p2/parser.hxx>
#include <c4/p2/lex/lexer.hxx>
#include <c4rt2c/ast2_ir_emitter.hxx>

#include <c4rt2c/source_file.hxx>

#include <libassert/assert.hpp>

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

#include <lyra/lyra.hpp>

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

template<class It, class S = It>
void
dump_ast(It begin, S end, std::ostream& out) {
	c4::ast_dumper dumper(out);
	std::for_each(
		std::move(begin), std::move(end),
		[&dumper, &out](const auto& expr) {
			expr->accept(dumper);
			out << "\n";
		}
	);
}

template<class It, class S = It>
void
dump_cps(It begin, S end, std::ostream& out) {
	// c4::ast3_dumper dumper(out);
	// std::for_each(
	// std::move(begin), std::move(end),
	// [&mapper, &dumper, &out](const auto& expr) {
	// expr->accept(mapper);
	// if (auto* e = mapper.result()) {
	// dumper.dump(e);
	// out << "\n";
	// }
	// }
	// );
}

void
initialize_targets();

int
main(int argc, const char** argv) {
	std::filesystem::path out_path;
	std::filesystem::path src_path;
	std::string target_arch;
	std::string dump_type;
	bool show_help = false;
	bool no_color_output = true;
	bool debug_trace = false;

	const auto cli = lyra::cli()
	                 | lyra::arg(src_path, "source")("The C4 source file to compile.").required()
	                 | lyra::help(show_help).description(
		                 "Compile a C4 script into an object file."
	                 )(
		                 "Do not compile, print help and exit."
	                 )
	                 | lyra::opt(out_path, "output")["-o"]["--output"](
		                 "The name of the output file. When -d is set, STDOUT if `-'."
	                 )
	                 | lyra::opt(dump_type, "dump")["-d"]["--dump"](
		                 "Do not compile, dump code instead. [AST, IR, ASM]"
	                 ).choices("AST", "IR", "ASM")
	                 | lyra::opt(target_arch, "target arch triplet")["-T"]["--target"](
		                 "The target triplet to produce the binary for."
	                 )
	                 | lyra::opt(debug_trace)["-D"]["--debug-trace"](
		                 "Generate code to bypass TCO allowing manual debugging,"
		                 "while breaking guarantees of computability"
	                 )
	                 | lyra::opt(no_color_output)["-C"]["--no-color"](
		                 "Disable color diagnostic output to STDERR. (Not yet implemented.)"
	                 )
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
	if (out_path.empty()) {
		out_path = src_path;
		out_path.replace_extension(".o");
	}

	src_path = absolute(src_path);
	c4::ast2::ast_context ast_context;
	c4::p2::lexer lexer(src_path.string(), src.begin(), src.end());
	c4::diagnostics_engine diagnostics_engine{stderr, !no_color_output};
	c4::p2::parser parser(ast_context, diagnostics_engine, std::move(lexer));

	try {
		const auto script = parser.parse_script();
		if (diagnostics_engine.errored())
			return 1;

		if (dump_type == "AST") {
			auto outstrm = open_outstream(out_path);
			dump_ast(script.begin(), script.end(), *outstrm);
			return 0;
		}

		initialize_targets();

		llvm::LLVMContext context;
		llvm::SMDiagnostic diag;
		const auto module_id = src_path.string();
		llvm::Module module(module_id, context);
		module.setSourceFileName(module_id);
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

		const auto machine = std::unique_ptr<llvm::TargetMachine>(
			target->createTargetMachine(target_triple, "generic", "", {}, llvm::Reloc::PIC_)
		);
		module.setDataLayout(machine->createDataLayout());
		module.setTargetTriple(target_triple);

		llvm::FunctionPassManager fn_pm;
		llvm::LoopAnalysisManager loop_am;
		llvm::FunctionAnalysisManager fn_am;
		llvm::ModuleAnalysisManager mod_am;
		llvm::CGSCCAnalysisManager cgscc_am;
		llvm::PassInstrumentationCallbacks pass_ic;
		llvm::StandardInstrumentations si(context, false);
		si.registerCallbacks(pass_ic, &mod_am);

		llvm::PassBuilder pass_builder;
		pass_builder.registerModuleAnalyses(mod_am);
		pass_builder.registerFunctionAnalyses(fn_am);
		pass_builder.crossRegisterProxies(loop_am, fn_am, cgscc_am, mod_am);

		c4rt2c::runtime_emitter rt_emitter(context, module, builder);
		rt_emitter.set_debug_trace(debug_trace);

		c4rt2c::ast2_ir_emitter ir(context, module, builder, std::move(rt_emitter));
		ir.init(src_path);

		ir.declare_symbols(ast_context);
		std::ranges::for_each(script, [&ir](const auto& expr) { expr->accept(ir); });

		ir.finalize();

		if (dump_type == "IR") {
			std::string dump;
			llvm::raw_string_ostream os(dump);
			module.print(os, nullptr);
			*open_outstream(out_path) << dump;
			return 0;
		}
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
