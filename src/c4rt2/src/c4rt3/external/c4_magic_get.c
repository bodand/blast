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
 * Originally created: 2026-08-17.
 *
 * src/c4rt2/src/c4rt3/external/c4_magic_get --
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <c4rt3/c4rt.h>
#include <c4rt3/external.h>

struct search_key {
	char* name;
	uint32_t arity;
};

static int
magic_cmp(const void* vkey, const void* ventry) {
	const struct search_key* key = vkey;
	const struct c4_magic_entry* entry = ventry;

	const int namecmp = strcmp(key->name, entry->name);
	if (namecmp != 0) return namecmp;

	return key->arity - entry->arity;
}

void
(*c4_magic_get(struct c4_external_magic* magic,
               char* name,
               uint32_t n))() {
	struct search_key key = { name, n };

	struct c4_magic_entry* entry = bsearch(&key,
	                                       magic->entries,
	                                       magic->entries_sz,
	                                       sizeof(struct c4_magic_entry),
	                                       magic_cmp);
	if (!entry) return 0;

	return entry->fn;
}

