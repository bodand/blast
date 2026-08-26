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
 * src/c4rt2/src/c4rt3/c4_datum_coerce_double --
 *   
 */

#if __has_include(<float.h>)
#	include <float.h>
#else
#	include <math.h>
#endif
#include <c4rt3/c4rt.h>

#include "internal_type.h"

#ifndef NAN
#	define NAN (0.0/0.0)
#endif

int
c4_datum_coerce_double(const c4_datum datum, double* const out) {
	if (!out) return 0;

	switch (c4_datum_type_of(datum)) {
	case C4_Float:
		return c4_datum_get_double(datum, out);
	case C4_Nil:
		*out = NAN;
		break;
	case C4_Integer:
		*out = (double)datum_int(datum);
		break;
	case C4_String:
		*out = (double)datum_str_sz(datum);
		break;
	case C4_Thunk:
	case C4_Block:
		*out = (double)(int64_t)datum_blk(datum);
		break;
	case C4_External:
		*out = (double)(int64_t)datum_ext(datum);
		break;
	}

	return 0;
}
