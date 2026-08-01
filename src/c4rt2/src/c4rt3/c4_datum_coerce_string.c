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
 * Originally created: 2026-08-01.
 *
 * src/c4rt2/src/c4rt3/c4_datum_coerce_string --
 *   
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <gc/gc.h>

#include <ryu/ryu.h>

#include <c4rt3/c4rt.h>

#include "internal_type.h"
#include "ska/fmt.h"

static int
flt_to_str(const double d,
           char** const out,
           size_t* const out_sz) {
	// don't allocate if we are just going to throw the result away
	char tmp[32];
	char* const buf = out ? GC_malloc(32) : tmp;
	if (!buf) return -(errno = ENOMEM);

	const int written = d2s_buffered_n(d, buf);
	buf[written] = '\0';

	if (out) *out = buf;
	if (out_sz) *out_sz = written;
	return 0;
}

static int
int_to_str(const int64_t i,
           char** const out,
           size_t* const out_sz) {
	char tmp[INT64_FMT + 1];
	char* const buf = out ? GC_malloc(INT64_FMT + 1) : tmp;
	if (!buf) return -(errno = ENOMEM);

	const size_t written = int64_fmt_generic(buf, i, 10);
	buf[written] = '\0';

	if (out) *out = buf;
	if (out_sz) *out_sz = written;
	return 0;
}

/*                 (   0x  <int-hex>     )   \0 */
#define BLOCK_FMT (1 + 2 + UINT64_XFMT + 1 + 1)

static int
blk_to_str(const uint64_t i,
           char** const out,
           size_t* const out_sz) {
	char tmp[BLOCK_FMT];
	char* const buf = out ? GC_malloc(BLOCK_FMT) : tmp;
	if (!buf) return -(errno = ENOMEM);

	memcpy(buf, "(0x", 3);
	const size_t written = uint64_fmt_generic(buf + 3, i, 16);
	memcpy(buf + 3 + written, ")\0", 2);

	if (out) *out = buf;
	if (out_sz) *out_sz = 1 + 2 + written + 1;
	return 0;
}

static char empty_str[] = "";

int
c4_datum_coerce_string(const c4_datum datum,
                       char** const out,
                       size_t* const out_sz) {
	switch (c4_datum_type_of(datum)) {
	case C4_String:
		return c4_datum_get_string(datum, out, out_sz);
	case C4_Nil:
		if (out) *out = empty_str;
		if (out_sz) *out_sz = 0;
		break;
	case C4_Integer:
		return int_to_str(datum_int(datum), out, out_sz);
	case C4_Float:
		return flt_to_str(datum_flt(datum), out, out_sz);
	case C4_Thunk:
	case C4_Block:
		return blk_to_str((uint64_t)datum_blk(datum), out, out_sz);
	}

	return 0;
}
