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
 * src/c4rt/include/c4rt/string --
 *   The C4 runtime's string related functionality.
 */
#ifndef C4RT_STRING_H
#define C4RT_STRING_H

#include <c4rt/api.h>

typedef struct c4rt_string_pool_* c4rt_string_pool;
typedef const char* c4rt_string;

/**
 * Copies a C-string into a newly allocated RT-string.
 */
C4RT_API c4rt_string
c4rt_str_from_cstring(c4rt_string_pool pool, const char* cstr);

/**
 * Copies an RT-string into a newly allocated RT-string.
 */
C4RT_API c4rt_string
c4rt_str_copy(c4rt_string_pool pool, c4rt_string rtstr);

/**
 * Returns a dynamically allocated C-string from a given RT-string.
 * Needs to be deallocated using free(3).
 */
C4RT_API char*
c4rt_str_cstr(c4rt_string rtstr);

/**
 * Returns the length in bytes of the given string. This is an O(1) operation
 * not O(n) like strlen(3).
 * Note that UTF-8 sequences could skew this value from the real "rendered"
 * character length.
 */
C4RT_API size_t
c4rt_str_length(c4rt_string rtstr);

/**
 * Releases a handle on an RT-string.
 */
C4RT_API void
c4rt_str_dealloc(c4rt_string rtstr);

/**
 * Allocates a string-pool to manage strings with in C4RT.
 */
C4RT_API c4rt_string_pool
c4rt_pool_allocate();

/**
 * Deallocates a string-pool. All RT-strings within this pool immediately
 * become dangling.
 */
C4RT_API void
c4rt_pool_deallocate(c4rt_string_pool pool);

/**
 * Checks if deallocating this pool would produce zombie RT-strings.
 */
C4RT_API bool
c4rt_pool_has_residents(c4rt_string_pool pool);

#endif
