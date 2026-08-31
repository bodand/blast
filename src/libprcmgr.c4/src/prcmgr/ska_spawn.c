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
 * Originally created: 2026-08-30.
 *
 * src/libprcmgr.c4/src/prcmgr/ska_spawn --
 *   
 */

#include <assert.h>
#include <c4rt3/c4rt.h>

#include <c4/array.h>

#include <skalibs/genalloc.h>
#include <skalibs/cspawn.h>

struct strview {
	char* s;
	size_t sz;
};

extern char** environ;

c4_let_native(ska_spawn)(
	c4_datum file,
	c4_datum argv_array_d,
	c4_datum envp,
	c4_datum flags,
	c4_datum fa_array
) {
	(void)fa_array;

	struct strview filev;
	c4_datum_coerce_string(file, &filev.s, &filev.sz);

	genalloc argv = GENALLOC_ZERO;
	c4_array argv_array;
	c4_datum_get_array(argv_array_d, &argv_array);
	genalloc_ready(const char*, &argv, argv_array->len + 1);

	for (size_t i = 0; i < argv_array->len; ++i) {
		const char* argv_str;
		c4_datum_coerce_string(argv_array->data[i],
		                       &argv_str,
		                       0);
		genalloc_append(const char*, &argv, &argv_str);
	}
	genalloc_append(const char*, &argv, &(const char*){0});

	char* envp_str;
	c4_datum_coerce_string(envp, &envp_str, 0);
	const char* const* env = (const char* const*)envp_str;

	uint64_t flags_u64;
	c4_datum_coerce_int64(flags, (int64_t*)&flags_u64);

	assert(flags_u64 < UINT16_MAX);
	const uint16_t flags_u16 = (uint16_t)flags_u64;

	cspawn_fileaction* fa = 0;
	size_t fa_sz = 0;

	const pid_t pid = cspawn(filev.s,
	                         genalloc_s(const char*, &argv),
	                         environ,
	                         flags_u16,
	                         fa,
	                         fa_sz);

	if (pid == 0) {
		c4_datum nil;
		c4_datum_from_nil(&nil);
		return nil;
	}

	c4_datum pid_d;
	c4_datum_from_int64(pid, &pid_d);
	return pid_d;
}
