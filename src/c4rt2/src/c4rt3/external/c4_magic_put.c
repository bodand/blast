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
 * src/c4rt2/src/c4rt3/external/c4_magic_put --
 */

#include <assert.h>
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

static size_t 
lower_bound(const void *key,
            const void *base,
            size_t n,
            size_t size,
            int (*cmp)(const void *, const void *)) {
	const char *p = base;
	size_t lo = 0, hi = n;

	while (lo < hi) {
		size_t mid = lo + (hi - lo) / 2;
		if (cmp(key, p + mid * size) > 0) {
			lo = mid + 1;
		}
		else {
			hi = mid;
		}
	}
	return lo;
}

void
c4_magic_put(struct c4_external_magic* magic,
             char* name,
             uint32_t n,
             void(*fn)()) {
	struct search_key key = { name, n };

	size_t insert_to = lower_bound(&key,
	                               magic->entries,
	                               magic->entries_sz,
	                               sizeof(struct c4_magic_entry),
	                               magic_cmp);
	if (magic->entries_sz > 0 
	    && magic_cmp(&key, &magic->entries[insert_to]) == 0) {
		magic->entries[insert_to].fn = fn;
		return;
	}

	magic->entries = realloc(magic->entries, magic->entries_sz + 1);
	assert(magic->entries);

	memmove(magic->entries + insert_to + 1, 
	        magic->entries + insert_to,
	        sizeof(struct c4_magic_entry) * (magic->entries_sz - insert_to));

	magic->entries[insert_to].name = name;
	magic->entries[insert_to].arity = n;
	magic->entries[insert_to].fn = fn;
	++magic->entries_sz;
}

