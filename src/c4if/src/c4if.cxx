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
 * Originally created: 2026-08-23.
 *
 * src/c4if/src/c4if --
 *   
 */

#include <cassert>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <string_view>

#include <sgetopt/opts.hxx>
#include <sgetopt/sgetopt.h>

#include <c4/diagnostic.hxx>
#include <c4/p2/included_parser.hxx>
#include <c4/p2/source_resolver.hxx>

#include <c4/ast2/ast_context.hxx>
#include <c4/p2/archive_parser.hxx>

using namespace std::literals;
namespace fs = std::filesystem;

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

	using vis_t = enum c4::ast2::let_expression::visibility;

	vis_t
	match_visibility(std::string_view optarg) {
		static std::vector<std::pair<std::string_view, vis_t>> words = {
			{"+"sv, static_cast<vis_t>('+')},
			{"public"sv, static_cast<vis_t>('+')},
			{"~"sv, static_cast<vis_t>('~')},
			{"internal"sv, static_cast<vis_t>('~')},
			{"-"sv, static_cast<vis_t>('-')},
			{"private"sv, static_cast<vis_t>('-')},
			{"all"sv, static_cast<vis_t>('-')},
		};

		const auto prefixes = [&optarg](const auto& word) {
			return word.first.starts_with(optarg);
		};

		const auto vises = std::ranges::count_if(words, prefixes);
		if (vises == 0) {
			std::cerr << "fatal: unknown visibility level: " << optarg << "\n";
			usage();
		}
		if (vises > 1) {
			std::cerr << "fatal: ambiguous visibility level: " << optarg << "\n";
			usage();
		}

		const auto it = std::ranges::find_if(words, prefixes);
		assert(it != words.end() && "somehow the value evaporated");

		return it->second;
	}

	struct generic_parser {
		std::vector<c4::ast2::let_expression*>
		parse_source(const fs::path& src_path) const {
			const auto src = _resolver.open_source(src_path);
			c4::p2::included_parser parser(_ast_context, _diag, _resolver, src);

			auto ret = parser.parse_global_let();
			std::ranges::for_each(ret, [&](auto& expr) {
				expr->lift_to_context(_ast_context);
			});

			return ret;
		}

		std::vector<c4::ast2::let_expression*>
		parse_archive(const fs::path& ar_path) const {
			const auto ar = _resolver.open_archive(ar_path);
			c4::p2::archive_parser parser(_ast_context, _diag, ar);

			auto ret = parser.parse();
			std::ranges::for_each(ret, [&](auto& expr) {
				expr->lift_to_context(_ast_context);
			});
			return ret;
		}

		c4::ast2::ast_context& _ast_context;
		c4::diagnostics_engine& _diag;
		c4::p2::source_resolver& _resolver;
	};
}

int
main(int argc, const char* const* argv) try {
	argv0 = argv[0];
	c4::diagnostics_engine diag(stderr);
	c4::p2::source_resolver resolver(diag);
	c4::ast2::ast_context ast_context;
	const generic_parser gen{ast_context, diag, resolver};

	auto action = &generic_parser::parse_source;

	auto filter_vis = vis_t::v_internal;

	subgetopt opts = SUBGETOPT_ZERO;
	opts.prog = argv0;
	for (int opt;
	     (opt = subgetopt_r(argc, argv, "ahI:v:", &opts)) != -1;) {
		switch (static_cast<char>(opt)) {
		case 'a':
			action = &generic_parser::parse_archive;
			break;
		case 'I':
			if (opts.arg[0] == '\0') {
				resolver.reset_path();
				break;
			}
			resolver.push_path(opts.arg);
			break;
		case 'v':
			filter_vis = match_visibility(opts.arg);
			break;
		default:
		case 'h':
			usage();
		}
	}

	argc -= opts.ind;
	argv += opts.ind;
	if (argc != 1) usage();

	const auto src_path = std::filesystem::absolute(argv[0]);

	for (const auto lets = (gen.*action)(src_path);
	     const auto& let : lets) {
		if (!let->seen_by(filter_vis)) continue;
		std::println(std::cout, "{}", let->pretty());
	}

	if (diag.errored()) return 1;
}
catch (std::exception& ex) {
	std::cerr << "fatal: " << ex.what() << "\n";
	return 111;
}
catch (...) {
	std::cerr << "fatal: weird error?\n";
	return 101;
}
