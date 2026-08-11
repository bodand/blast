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
 * Originally created: 2026-08-08.
 *
 * src/std4/src/std/array --
 *   
 */
#ifndef BLAST_ARRAY_H
#define BLAST_ARRAY_H

#include <c4rt3/c4rt.h>

struct c4_array_t {
	c4_datum* data;
	size_t len;
	size_t cap;
};

typedef struct c4_array_t* c4_array;

c4_extern uint32_t
c4_array_external_typeid();

c4_extern c4_array
c4_array_new(size_t len);

c4_extern c4_array
c4_array_ensure(c4_array arr, size_t len);

c4_extern c4_array
c4_array_ensure_more(c4_array arr, size_t len);

#define \
c4_datum_from_array(arr, out) c4_datum_from_external(arr, c4_array_external_typeid(), out)

#define \
c4_datum_get_array(datum, out) do {                                            \
		void* raw = NULL;                                                        \
		c4_datum_get_external(datum, c4_array_external_typeid(), &raw);          \
		*out = (c4_array)raw; \
	} while (0)

#endif
