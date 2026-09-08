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
 * Originally created: 2026-08-31.
 *
 * src/libsgetopt/inc/sgetopt/opts_helper --
 *   A wrapper around subgetopt_r
 */
#ifndef BLAST_OPTS_HELPER_HXX
#define BLAST_OPTS_HELPER_HXX

#include <c4/diagnostic.hxx>

#include <sgetopt/sgetopt.h>

namespace sopts {
	struct opts_helper {
		opts_helper(int& argc, const char* const*& argv);

		char
		step(std::string_view ops);

		c4::position
		here() const;

		std::string_view
		arg() const { return std::string_view(opts.arg); }

		void
		shift(const std::optional<int> by = {}) const {
			if (!by) {
				_argc -= opts.ind;
				_argv += opts.ind;
			}
			else {
				_argc -= *by;
				_argv += *by;
			}
		}

	private:
		std::pair<size_t, size_t>
		arg_indices() const {
			auto index = nth_arg_pos(opts.pos, opts.ind);
			auto index_end = index;
			if (opts.arg) {
				--index_end;
				index -= std::strlen(opts.arg);
				do --index;
				while (_merged[index - 1] != option_name());
			}
			while (_merged[index - 1] == option_name()
			       || _merged[index - 1] == '-')
				--index;
			++index;

			return {index, index_end};
		}

		std::size_t
		nth_arg_pos(unsigned pos, int ind) const;

		char
		option_name() const noexcept {
			if (!opts.problem) return _option[0];
			if (opts.problem == '-') return _option[0];
			return opts.problem;
		}

		char _option[2] = {'?', 0};
		int& _argc;
		const char* const*& _argv;
		subgetopt opts = SUBGETOPT_ZERO;
		std::string _merged;
	};
}

#endif
