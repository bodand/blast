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
 * Originally created: 2026-08-19.
 *
 * src/blast/src/blast/blast_match_ast --
 *   
 */

#include <c4rt3/c4rt.h>

#include <chrono>
#include <cstdint>
#include <cstdint>
#include <limits>
#include <limits>
#include <random>

#include <gc/gc.h>
#include <thread>
#include <thread>

namespace {
	struct pcg32_random_t {
		uint64_t state;
		uint64_t inc;
	};

	uint32_t 
	pcg32_random_r(pcg32_random_t* rng) {
		 uint64_t oldstate = rng->state;
		 // Advance internal state
		 rng->state = oldstate * 6364136223846793005ULL + (rng->inc|1);
		 // Calculate output function (XSH RR), uses old state for max ILP
		 uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
		 uint32_t rot = oldstate >> 59u;
		 return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
	}

	thread_local pcg32_random_t _random_state { 0x853c49e6748fea9bULL, 0xda3e39cb94b95bdbULL };

	struct random_gen {
		using result_type = uint32_t;

		static constexpr uint32_t 
		max() noexcept { return std::numeric_limits<uint32_t>::max(); }
		static constexpr uint32_t
		min() noexcept { return std::numeric_limits<uint32_t>::min(); }

		uint32_t
		operator()() {
			return pcg32_random_r(state);
		}
		pcg32_random_t* state = &_random_state;
	};
}

c4_let_native(blast_new_unique_binding)() {
	const auto now = std::chrono::high_resolution_clock::now();
	std::uniform_int_distribution<int> dist(0, 25);

	random_gen g;

	std::array<char, 8> randbuf;
	std::generate_n(randbuf.begin(), size(randbuf), [&]() { 
				return 'A'+(char)dist(g); 
				});

	std::string_view randstr(randbuf.begin(), randbuf.end());
	std::string str = std::format("_{}{}{}", now.time_since_epoch().count(), std::this_thread::get_id(), randstr);

	c4_datum out;
	c4_datum_from_string_sz(str.data(), str.size(), &out);
	return out;
}

