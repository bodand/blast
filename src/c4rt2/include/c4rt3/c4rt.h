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
 * Originally created: 2026-07-31.
 *
 * src/c4rt2/include/c4rt3/c4rt --
 *   Support functions and macros for writing C4 native functions.
 */
#ifndef BLAST_C4RT_HXX
#define BLAST_C4RT_HXX

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

struct c4_datum_t;

typedef struct c4_datum_t* c4_datum;

enum c4_datum_type {
	C4_Integer  = 0,
	C4_Thunk    = 1,
	C4_Float    = 3,
	C4_String   = 4,
	C4_Block    = 5,
	C4_Nil      = 6,
	C4_External = 7
};
typedef enum c4_datum_type c4_datum_type;

#define c4_native_prefix c4sym_

#define c4_cat_impl(x, y) x##y
#define c4_cat(x, y) c4_cat_impl(x, y)

#ifdef __cplusplus
#  define c4_extern extern "C"
#else
#  define c4_extern extern
#endif

#define c4_let_native(sym) \
	c4_extern c4_datum \
	c4_cat(c4_native_prefix, sym)

c4_extern c4_datum_type
c4_datum_type_of(c4_datum datum);

c4_extern bool
c4_datum_is_nil(c4_datum datum);

c4_extern int
c4_datum_get_int64(c4_datum datum, int64_t* out);

c4_extern int
c4_datum_get_double(c4_datum datum, double* out);

c4_extern int
c4_datum_get_string(c4_datum datum, char** out, size_t* out_sz);

c4_extern int
c4_datum_get_external(c4_datum datum, uint64_t exttype, void** out);

c4_extern int
c4_datum_coerce_int64(c4_datum datum, int64_t* out);

c4_extern int
c4_datum_coerce_double(c4_datum datum, double* out);

c4_extern int
c4_datum_coerce_string(c4_datum datum, char** out, size_t* out_sz);

#define \
c4_datum_from_string(str, out) c4_datum_from_string_sz(str, strlen(str), out)

c4_extern int
c4_datum_from_string_sz(const char* str, size_t str_sz, c4_datum* out);

c4_extern int
c4_datum_from_int64(int64_t val, c4_datum* out);

c4_extern int
c4_datum_from_double(double val, c4_datum* out);

c4_extern int
c4_datum_from_boolean(bool val, c4_datum* out);

c4_extern int
c4_datum_from_nil(c4_datum* out);

c4_extern int
c4_datum_from_external(void* val, uint64_t exttype, c4_datum* out);

c4_extern int
c4_datum_cmp(c4_datum a, c4_datum b);

c4_extern uint64_t
c4_next_external_typeid(void);

#endif
