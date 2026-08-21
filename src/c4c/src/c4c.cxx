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

#include <filesystem>
#include <filesystem>
#include <ios>
#include <ios>
#include <skalibs/buffer.h>
#include <utility>
#include <utility>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <algorithm>
#include <charconv>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <system_error>

#include <fmt/format.h>

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
#include <llvm/IR/LegacyPassManager.h>
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

#include <sgetopt/sgetopt.h>

using namespace std::literals;

namespace {
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

	void
	initialize_targets();

	#define argdesc(flag, arg, ...) "   " #flag "  "  << std::setw(w) << #arg << "   " #__VA_ARGS__ "\n"

	[[noreturn]] void
	usage(const char* argv0) {
		constexpr int w = 7;
		std::cerr << "usage: " << argv0 << " [-dEghIOoT] <source>\n"
				<< "\n"
				<< "options:\n"
				<< std::left // don't need to restore, we are exiting imminently
				<< argdesc(-d, type, Set dump type written to output. By default it is object code.)
				<< argdesc(-E, , Compile source as executable entrypoint.)
				<< argdesc(-g, debug, Enable debugging flag for compiling this TU. See c4c-debug(7).)
				<< argdesc(-h, , Print this help and exit 100.)
				<< argdesc(-I, dir, Add dir for finding C4 library archives.)
				<< argdesc(-O, level, Set optimization level. Values are 0-3.)
				<< argdesc(-o, file, The file to use as output. A bare - means STDOUT. Defaults to source with .o suffix.)
				<< argdesc(-T, triplet, Set target triplet to trp. Same format as LLVM.);
		exit(100);
	}

	template<class... Args>
	[[noreturn]] void
	die(const int e,
	    fmt::format_string<Args...> fmt, Args&&... args) {
		std::cerr << fmt::format(fmt, std::forward<Args>(args)...);
		exit(e);
	}

	std::optional<std::string>
	make_libinit_name(const bool build_entrypoint,
	                  const std::filesystem::path& src) {
		if (build_entrypoint) return {};

		const auto rel = relative(src).make_preferred().replace_extension();
		auto rel_str = rel.string();
		std::ranges::transform(rel_str, begin(rel_str), [](const char c) {
			if (c == std::filesystem::path::preferred_separator) return '_';
			if (c == '.') return '_';
			return c;
		});

		return rel_str + "_init";
	}
}

int
main(int argc, const char* const* argv) {
	const auto argv0 = argv[0];

	std::filesystem::path out_path;
	std::filesystem::path src_path;
	std::string target_arch;
	std::string dump_type;
	int opt_level = 0;

	bool debug_trace = false;
	bool debug_gc = false;

	bool build_entrypoint = false;

	subgetopt opts = SUBGETOPT_ZERO;
	opts.prog = argv[0];
	for (int opt;
	     (opt = subgetopt_r(argc, argv, "d:Eg:hI:O:o:T:", &opts)) != -1;) {
		switch (static_cast<char>(opt)) {
		case 'd': {
			dump_type = opts.arg;
			if (dump_type == "AST") break;
			if (dump_type == "IR") break;
			if (dump_type == "ASM") break;
			if (dump_type == "LTO") break;
			die(100, "{}: fatal: invalid argument for {}: expected AST, IR, ASM, or LTO", argv0, "-d");
		}
		case 'E': {
			build_entrypoint = true;
			break;
		}
		case 'g': {
			std::string_view tmp = opts.arg;
			if (tmp == "call-trace") {
				debug_trace = true;
				break;
			}
			if (tmp == "debug-gc") {
				debug_gc = true;
				break;
			}
			die(100, "{}: fatal: invalid argument for {}: see c4c-debug(7) for valid values", argv0, "-g");
		}
		case 'I': {
			break; // TODO
		}
		case 'O': {
			auto [ptr, ec] = std::from_chars(opts.arg, opts.arg + std::strlen(opts.arg), opt_level);
			if (ec != std::errc{}
			    || *ptr != '\0') {
				die(100, "{}: fatal: invalid argument for {}: {}\n", argv[0], "-O", std::make_error_code(ec).message());
			}
			break;
		}
		case 'o': {
			out_path = opts.arg;
			break;
		}
		case 'T': {
			target_arch = opts.arg;
			break;
		}

		case 'h':
		default:
			usage(argv0);
		}
	}

	argc -= opts.ind;
	argv += opts.ind;
	if (argc != 1) usage(argv0);

	src_path = argv[0];

	auto opt = llvm::OptimizationLevel::O0;
	switch (std::max(std::min(opt_level, 3), 0)) {
	case 0: break;
	case 1: opt = llvm::OptimizationLevel::O1;
		break;
	case 2: opt = llvm::OptimizationLevel::O2;
		break;
	case 3: opt = llvm::OptimizationLevel::O3;
		break;
	}

	c4c::source_file src(src_path);
	if (out_path.empty()) {
		out_path = src_path;
		out_path.replace_extension(".o");
	}

	src_path = absolute(src_path);
	c4::ast2::ast_context ast_context;
	c4::p2::lexer lexer(src_path.string(), src.begin(), src.end());
	c4::diagnostics_engine diagnostics_engine{stderr};
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
		const auto llvm_triple = llvm::Triple(target_triple);

		std::string target_error;
		const auto target = llvm::TargetRegistry::lookupTarget(llvm_triple, target_error);
		if (!target) {
			std::cerr << "fatal: " << target_error << "\n";
			return 2;
		}

		const auto machine = std::unique_ptr<llvm::TargetMachine>(
			target->createTargetMachine(llvm_triple, "generic", "", {}, llvm::Reloc::PIC_)
		);
		module.setDataLayout(machine->createDataLayout());
		module.setTargetTriple(llvm_triple);

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

		const bool decl_only_rt = !build_entrypoint;
		c4rt2c::runtime_emitter rt_emitter(context, module, builder, decl_only_rt);
		rt_emitter.set_debug_trace(debug_trace);
		rt_emitter.set_memory_debug(debug_gc);

		const auto library_init_fn = make_libinit_name(build_entrypoint, src_path);
		c4rt2c::ast2_ir_emitter ir(module, builder, std::move(rt_emitter), library_init_fn);
		ir.init(src_path);

		ir.declare_symbols(ast_context);
		std::ranges::for_each(script, [&ir](const auto& expr) { expr->accept(ir); });

		ir.finalize();

		llvm::ModulePassManager mod_pm;
		if (opt_level == 0) {
			mod_pm = pass_builder.buildO0DefaultPipeline(opt);
		}
		else {
			mod_pm = pass_builder.buildPerModuleDefaultPipeline(opt);
		}
		mod_pm.run(module, mod_am);

		if (dump_type == "IR") {
			std::string dump;
			llvm::raw_string_ostream os(dump);
			module.print(os, nullptr);
			*open_outstream(out_path) << dump;
			return 0;
		}

		std::error_code ec;
		llvm::raw_fd_ostream out(out_path.string(), ec);
		if (ec) throw std::system_error(ec);

		llvm::legacy::PassManager pm;
		const auto ftype = llvm::CodeGenFileType::ObjectFile;

		if (machine->addPassesToEmitFile(pm, out, nullptr, ftype)) {
			std::cerr << "\033[31mfatal:\033[0m target machine cannot produce object files. Bummer.\n";
			return 100;
		}

		pm.run(module);
		out.flush();
	}
	catch (const std::exception& e) {
		std::cerr << "\033[31mfatal:\033[0m " << e.what() << "\n";
		return 111;
	}
}

namespace {
	void
	initialize_targets() {
		llvm::InitializeAllTargetInfos();
		llvm::InitializeAllTargets();
		llvm::InitializeAllTargetMCs();
	}
}