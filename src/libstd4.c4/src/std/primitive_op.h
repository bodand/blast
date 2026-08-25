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
 * src/std4/src/std/primitive_op --
 *   
 */
#ifndef BLAST_PRIMITIVE_OP_H
#define BLAST_PRIMITIVE_OP_H

#define c4_primitive_op(p, op) \
	c4_let_native(p)(const c4_datum a, const c4_datum b) { \
		double a_dbl, b_dbl; \
		bool have_a_dbl = 0 == c4_datum_get_double(a, &a_dbl); \
		bool have_b_dbl = 0 == c4_datum_get_double(b, &b_dbl); \
\
		int64_t a_int, b_int; \
		if (!have_a_dbl) { \
			c4_datum_coerce_int64(a, &a_int); \
			if (have_b_dbl) a_dbl = (double)a_int; \
		} \
		if (!have_b_dbl) { \
			c4_datum_coerce_int64(b, &b_int); \
			if (have_a_dbl) b_dbl = (double)b_int; \
		} \
\
		c4_datum ret; \
		if (have_a_dbl || have_b_dbl) { \
			c4_datum_from_double(op(a_dbl, b_dbl), &ret);\
		} \
		else { \
			c4_datum_from_int64(op(a_int, b_int), &ret); \
		}\
		return ret; \
	}

#endif
