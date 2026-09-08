/* blAST project
 *
 * Copyright (c) 2026 András Bodor <bodand@pm.me>
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
 * Originally created: 2026-08-27.
 *
 * src/c4ld/src/c4ld --
 *   
 */

#include <thread>
#include <c4/diagnostic.hxx>
#include <c4/p2/source_resolver.hxx>

#include <sgetopt/opts.hxx>
#include <sgetopt/sgetopt.h>
#include <sgetopt/opts_helper.hxx>

#include <llvm/Support/Process.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/VirtualFileSystem.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/TargetParser/Triple.h>

#include <lld/Common/Driver.h>
#include <lld/Common/Strings.h>
#include <lld/Common/Timer.h>

#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/DiagnosticIDs.h>
#include <clang/Driver/Compilation.h>
#include <clang/Driver/Driver.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>

LLD_HAS_DRIVER(elf)

LLD_HAS_DRIVER(coff)

LLD_HAS_DRIVER(macho)

LLD_HAS_DRIVER(mingw)

using namespace std::literals;

namespace {
	#define argdesc(flag, arg, ...) "   " #flag "  "  << std::setw(w) << #arg << "   " #__VA_ARGS__ "\n"

	[[noreturn]] void
	usage() {
		constexpr int w = 5;
		std::cerr << "usage: " << argv0 << " [-ahIv] <source>\n"
				<< "\n"
				<< "options: \n"
				argdesc(-a, , Process input source as C4 archive.)
				argdesc(-h, , Print this help and exit 100.)
				argdesc(-I, path, Append path to library search path.)
				argdesc(-v, vis, Set visibility level to extract at. +, [~], or -);
		exit(100);
	}

	template<std::integral I>
	bool
	try_parse(std::string_view str, I& out, const char** endptr = nullptr) {
		const auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), out);
		if (ec != std::errc{}) return false;
		if (endptr) *endptr = ptr;
		return true;
	}
}

int
main(int argc, const char* const* argv) {
	argv0 = argv[0];
	c4::diagnostics_engine diag(stderr);
	c4::p2::source_resolver resolver(diag);

	std::filesystem::path out_path = "a.out";
	std::optional<std::string> triplet_str;
	bool cpp = false;
	int opt_level = 1;

	bool dump_lld_call = false;
	bool dump_clang_call = false;

	c4::p2::invalid_file_source internal_src("<c4ld>");

	size_t jobs = std::thread::hardware_concurrency();
	std::vector<std::string> standard_libs{"std4", ":libc4rt3.a"};
	std::vector<std::string> libraries;

	std::string suggestion_str;
	sopts::opts_helper helper{argc, argv};
	while (char opt = helper.step("#+hI:j:L:l:Oo:S:T:t:")) {
		switch (opt) {
		case '#':
			if (!dump_lld_call) {
				dump_lld_call = true;
			}
			else {
				dump_clang_call = true;
			}
			break;
		case '+':
			cpp = true;
			break;
		case 'L': {
			auto pos = helper.here();
			diag.warning(helper.here(),
			             "the c4 linker uses -I for library search path: interpreting -L as -I")
			    .suggest([&pos, &suggestion_str]() -> c4::position&& {
				             suggestion_str = pos.expanded_range;
				             auto flag_idx = pos.col_number;
				             if (suggestion_str[flag_idx] != 'L') --flag_idx;
				             suggestion_str[flag_idx] = 'I';
				             pos.col_number = flag_idx + 1;
				             pos.col_number_end = flag_idx + 1;
				             pos.expanded_range = suggestion_str;
				             return std::move(pos);
			             }(),
			             "if you guessed by ld(1) behavior, just use -I instead");
			[[fallthrough]];
		}
		case 'l':
			if (const auto lib = resolver.resolve_link_library(helper.here(), helper.arg()))
				libraries.emplace_back(*lib);
			break;
		case 'I':
			if (helper.arg() == "-") {
				resolver.reset_path();
				break;
			}
			resolver.push_path(helper.arg());
			break;
		case 'j': {
			if (try_parse(helper.arg(), jobs)) break;
			diag.error(helper.here(),
			           "invalid number of jobs for -j, expected a number: {}",
			           helper.arg());
			break;
		}
		case 'O':
			opt_level = 2;
			break;
		case 'o':
			out_path = helper.arg();
			break;
		case 'S':
			if (helper.arg() == "no-std") {
				erase(standard_libs, "std4");
				break;
			}
			if (helper.arg() == "no-stdrt") {
				erase(standard_libs, ":c4rt3.a");
				break;
			}
			if (helper.arg() == "no-rt3") {
				erase(standard_libs, ":c4rt3.a");
				break;
			}
			if (helper.arg() == "none") {
				standard_libs.clear();
				break;
			}
			diag.error(helper.here(), "unrecognized option {} for -S", helper.arg());
			break;
		case 'T':
			triplet_str = helper.arg();
			break;
		case 't':
			if (helper.arg() == "exec" || helper.arg() == "executable") break;
			if (helper.arg() == "so" || helper.arg() == "dll") {
				diag.error(helper.here(), "dynamic library generation not implemented, sorry");
				break;
			}
			diag.error(helper.here(), "unrecognized option {} for -t", helper.arg());
			break;
		default:
			diag.error(helper.here(), "unrecognized option {}", opt);
			[[fallthrough]];
		case 'h':
			usage();
		}
	}

	std::ranges::for_each(standard_libs, [&](const auto& lib) {
		const auto resolved = resolver.resolve_link_library(c4::position::invalid_file_position(internal_src), lib);
		if (!resolved) return;

		libraries.emplace_back(*resolved);
	});

	if (diag.errored()) return 100;
	helper.shift();

	std::string error_buffer;
	llvm::raw_string_ostream err(error_buffer);

	clang::DiagnosticOptions diag_opts;
	diag_opts.ShowColors = llvm::sys::Process::StandardErrHasColors();
	err.enable_colors(diag_opts.ShowColors);

	auto printer = new clang::TextDiagnosticPrinter(err, diag_opts);
	const auto diag_ids = llvm::IntrusiveRefCntPtr(new clang::DiagnosticIDs);
	clang::DiagnosticsEngine clang_diag(diag_ids, diag_opts, printer);

	const auto target_triplet = triplet_str.value_or(llvm::sys::getDefaultTargetTriple());
	llvm::Triple triple(target_triplet);

	std::vector<std::string> clang_str_args;
	clang_str_args.push_back(argv0);
	clang_str_args.push_back("-fuse-ld=lld");
	if (cpp) clang_str_args.push_back("--driver-mode=g++");
	clang_str_args.push_back(std::format("-o{}", out_path.string()));
	std::ranges::copy(argv, argv + argc, std::back_inserter(clang_str_args));

	if (dump_clang_call) {
		std::ranges::copy(clang_str_args, std::ostream_iterator<std::string>(std::cout, " "));
		std::cout << "\n";
	}

	std::vector<const char*> clang_args(clang_str_args.size());
	std::ranges::transform(clang_str_args, clang_args.begin(), std::mem_fn(&std::string::c_str));

	clang::driver::Driver clang(argv0, target_triplet, clang_diag, "c4ld");
	const auto what_clang_would_do = clang.BuildCompilation(clang_args);
	if (!what_clang_would_do
	    || what_clang_would_do->containsError()
	    || clang_diag.hasErrorOccurred()) {
		std::cerr << error_buffer;
		return 111;
	}

	std::vector<std::string> str_args;
	str_args.push_back("ld.lld");
	str_args.push_back("--pie");
	str_args.push_back("--gc-sections");
	str_args.push_back("--icf=all");
	if (diag_opts.ShowColors) str_args.push_back("--color-diagnostics=always");
	str_args.push_back(std::format("-O{}", opt_level));

	for (auto job : what_clang_would_do->getJobs()) {
		str_args[0] = job.getExecutable();
		std::ranges::copy(job.getArguments(), std::back_inserter(str_args));
	}

	std::ranges::copy(libraries, std::back_inserter(str_args));
	str_args.push_back("-lgc");

	if (dump_lld_call) {
		std::ranges::copy(str_args, std::ostream_iterator<std::string>(std::cout, " "));
		std::cout << "\n";
	}

	std::vector<const char*> args(str_args.size());
	std::ranges::transform(str_args, args.begin(), std::mem_fn(&std::string::c_str));

	constexpr lld::DriverDef driver_gnu{.f = lld::Gnu, .d = lld::elf::link};
	constexpr lld::DriverDef driver_apple{.f = lld::Darwin, .d = lld::macho::link};
	constexpr lld::DriverDef driver_mingw{.f = lld::MinGW, .d = lld::mingw::link};
	constexpr lld::DriverDef driver_coff{.f = lld::WinLink, .d = lld::coff::link};
	constexpr std::array drivers = {
		driver_gnu, driver_apple, driver_mingw, driver_coff,
	};

	llvm::InitializeAllTargets();

	const auto [retCode, canRunAgain] = lld::lldMain(args,
	                                                 llvm::outs(), err,
	                                                 drivers);

	std::cerr << error_buffer;
	return retCode;
}
