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
 * Originally created: 2026-08-10.
 *
 * src/c4rt2/include/c4rt3/external --
 *   This file allows external types to register their own behaviors into
 *   C4_External typed data. It is named magic as homage to Perl's MAGIC
 *   struct of a similar purpose and because it cannot be manipulated from the
 *   C4 language itself.
 *
 *   The structure is effectively a dynamic vtable. Each type can emplace
 *   an amount of (name, n, fnptr) triplets and a C function can try to lookup 
 *   a function by name and arity. All argument types are always pointers, the
 *   thing they point to depends on the actual function.
 *
 *   Magic function names can be anything, except starting with "_c4" which
 *   are reserved for the C4 language's own shenanigans.
 *
 *   The current native magics understood by specific parts of C4's runtime:
 *   	_c4_cmp/2 -> compares 2 external c4_datum objects
 *   	_c4_stringify/1 -> makes a string out of a c4_datum object
 *
 *   Note that magic is not overallocated. This is because it likely is not
 *   used much.
 */
#ifndef C4RT3_EXTERNAL_H
#define C4RT3_EXTERNAL_H

#include <c4rt3/c4rt.h>

struct c4_magic_entry {
	char* name;
	uint32_t arity;
	void (*fn)(void);
};

struct c4_external_magic {
	struct c4_magic_entry* entries;
	size_t entries_sz;
};

#define C4_MAGIC_ZERO { 0, 0 }

void
c4_magic_put(struct c4_external_magic* magic,
             char* name,
             uint32_t n,
             void(*fn)(void));

void
(*c4_magic_get(struct c4_external_magic* magic,
               char* name,
               uint32_t n))(void);

struct c4_external_magic*
c4_register_magic(c4_datum d);

struct c4_external_magic*
c4_get_magic(c4_datum d);

#endif

