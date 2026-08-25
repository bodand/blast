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
 * Originally created: 2026-08-22.
 *
 * src/sgetopt/inc/sgetopt/opts --
 *   
 */
#ifndef BLAST_SGETOPT_OPTS_HXX
#define BLAST_SGETOPT_OPTS_HXX

#include <concepts>
#include <ostream>

extern "C" const char* argv0;

template<class... Args>
[[noreturn]] void
die(const int e,
	 std::format_string<std::string_view, Args...> fmt, Args&&... args) {
	std::println(std::cerr, fmt, std::string_view{argv0}, std::forward<Args>(args)...);
	exit(e);
}

[[noreturn]] inline void
argdie(std::string_view option, std::string_view message) {
	die(100, "{}: fatal: invalid argument for {}: {}\n", option, message);
}

template <std::integral I>
void
die_or_parse(std::string_view opt, std::string_view option_view, I& output) {
	if (auto [ptr, ec] = std::from_chars(option_view.data(),
	                                     option_view.data() + option_view.size(),
	                                     output);
		ec != std::errc{} || *ptr != '\0') {
		argdie(opt, std::make_error_code(ec).message());
	}
}

template <std::integral I>
void
die_or_parse(const char* option, I& output) {
	die_or_parse(std::string_view{option}, output);
}

#endif
