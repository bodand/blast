/* blAST project
 *
 * Copyright (c) 2025 András Bodor <bodand@pm.me>
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
 * Originally created: 2025-03-03.
 *
 * src/c4rt/include/c4rt/datum --
 *   
 */
#ifndef C4RT_DATUM_H
#define C4RT_DATUM_H

#ifdef __cplusplus
#  include <cstdint>
#else
#  include <stdint.h>
#  include <stdbool.h>
#endif

#include <c4rt/api.h>

typedef uint64_t c4_datum_t;

typedef enum c4_datum_type_ {
    C4_Float = 0x0, /* 0000 */
    C4_Block = 0x1, /* 0001 */
    C4_Integer = 0x2, /* 0010 */
    C4_String = 0x4, /* 0100 */
} c4_datum_type;

C4RT_API const c4_datum_t gC4_Empty_Block;

C4RT_API c4_datum_t
_C5print1(c4_datum_t d);

C4RT_API c4_datum_t
_C7println1(c4_datum_t d);

C4RT_API c4_datum_t
_C2if3(c4_datum_t cond, c4_datum_t yes, c4_datum_t no);

C4RT_API c4_datum_t
_C6readln0();

C4RT_API c4_datum_t
_C9str_empty1(c4_datum_t str);

C4RT_API void
c4rt_free(void* mem);

C4RT_API bool
c4rt_datum_remoteness_of(c4_datum_t datum);

C4RT_API c4_datum_type
c4rt_datum_type_of(c4_datum_t datum);

C4RT_API c4_datum_t
c4rt_datum_from_int32(int32_t i);

C4RT_API c4_datum_t
c4rt_datum_from_int64(int64_t i);

C4RT_API c4_datum_t
c4rt_datum_from_double(double d);

C4RT_API c4_datum_t
c4rt_datum_from_string(const char* s);

C4RT_API c4_datum_t
c4rt_datum_from_string_sz(const char* s, size_t s_sz);

C4RT_API c4_datum_t
c4rt_datum_from_proc(c4_datum_t (*proc)(),
                     unsigned arity,
                     unsigned context_size,
                     c4_datum_t* context);

C4RT_API void
c4rt_datum_free(c4_datum_t d);

C4RT_API int32_t
c4rt_datum_get_int32(c4_datum_t datum);

C4RT_API int64_t
c4rt_datum_get_intó4(c4_datum_t datum);

C4RT_API double
c4rt_datum_get_double(c4_datum_t datum);

C4RT_API const char*
c4rt_datum_get_string(c4_datum_t datum);

C4RT_API int32_t
c4rt_datum_coerce_int32(c4_datum_t datum);

C4RT_API int64_t
c4rt_datum_coerce_int64(c4_datum_t datum);

C4RT_API double
c4rt_datum_coerce_double(c4_datum_t datum);

/**
 * \brief Creates a new string from a given datum.
 *
 * Coerces the given datum into a newly allocated string value.
 * This value must be released using c4rt_free.
 **/
C4RT_API char*
c4rt_datum_coerce_string(c4_datum_t datum);

C4RT_API c4_datum_t
c4rt_datum_dup(c4_datum_t datum);

#endif
