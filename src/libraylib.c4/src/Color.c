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
 * Originally created: 2026-08-16.
 *
 * src/raylib/src/Color --
 *   
 */

#include <c4rt3/c4rt.h>

#include <raylib.h>
#include <gc/gc.h>

#include "types.h"

c4_let_native(Color)(
	const c4_datum r,
	const c4_datum g,
	const c4_datum b,
	const c4_datum a
) {
	int64_t r_i64, g_i64, b_i64, a_i64;
	c4_datum_coerce_int64(r, &r_i64);
	c4_datum_coerce_int64(g, &g_i64);
	c4_datum_coerce_int64(b, &b_i64);
	c4_datum_coerce_int64(a, &a_i64);

	Color* color = GC_NEW(Color);
	color->r = (uint8_t)r_i64;
	color->g = (uint8_t)g_i64;
	color->b = (uint8_t)b_i64;
	color->a = (uint8_t)a_i64;

	c4_datum out;
	c4_datum_from_Color(color, &out);
	return out;
}
