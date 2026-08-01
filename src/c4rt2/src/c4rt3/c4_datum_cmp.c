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
 * src/c4rt2/src/c4rt3/c4_datum_cmp --
 *   
 */

#include <string.h>

#include <c4rt3/c4rt.h>

#include "internal_type.h"

static int
int_cmp(const int64_t a, const int64_t b) {
	return a - b;
}

static int
float_cmp(const double a, const double b) {
	return a - b;
}

static int
string_cmp(const char* const a, const char* const b) {
	return strcmp(a, b);
}

static int
pointer_cmp(const void* const a, const void* const b) {
	return (int)((intptr_t)a - (intptr_t)b);
}

int
c4_datum_cmp(const c4_datum a, const c4_datum b) {
	const c4_datum_type a_type = a->type;
	const c4_datum_type b_type = b->type;

	if (a_type != b_type) return a_type - b_type;

	switch (a_type) {
	case C4_Integer: return int_cmp(datum_int(a), datum_int(b));
	case C4_Float: return float_cmp(datum_flt(a), datum_flt(b));
	case C4_String: return string_cmp(datum_str(a), datum_str(b));
	case C4_Nil: return 0;
	case C4_Thunk:
	case C4_Block:
		return pointer_cmp(datum_blk(a), datum_blk(b));
	}
	return 0;
}
