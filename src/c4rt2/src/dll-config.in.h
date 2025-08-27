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
 * src/c4rt/src/dll --
 *   
 */
#ifndef C4RT_DLL_CONFIG_H
#define C4RT_DLL_CONFIG_H

@PVOID_SIZE_CODE@

#ifdef __cplusplus
#  define C4_EXTERNC extern "C"
#else
#  define C4_EXTERNC
#endif

#cmakedefine C4RT2_USE_MIMALLOC
#ifdef C4RT2_USE_MIMALLOC
#  include <mimalloc.h>

#  define C4_MALLOC(...) mi_malloc(__VA_ARGS__)
#  define C4_MALLOC_SMALL(...) mi_malloc_small(__VA_ARGS__)
#  define C4_STRDUP(...) mi_strdup(__VA_ARGS__)
#  define C4_FREE(...) mi_free(__VA_ARGS__)
#  define C4_SMALL_ALLOC_SIZE MI_SMALL_SIZE_MAX
#else
#  include <stdlib.h>
#  include <string.h>

#  define C4_MALLOC(...) malloc(__VA_ARGS__)
#  define C4_MALLOC_SMALL(...) malloc(__VA_ARGS__)
#  define C4_STRDUP(...) strdup(__VA_ARGS__)
#  define C4_FREE(...) free(__VA_ARGS__)
#  define C4_SMALL_ALLOC_SIZE 0U
#endif

C4_EXTERNC inline void*
C4_ALLOCATE(const size_t bytes) {
    if (bytes < C4_SMALL_ALLOC_SIZE) return C4_MALLOC_SMALL(bytes);
    return C4_MALLOC(bytes);
}

#define C4_NEW(type, count) C4_ALLOCATE(sizeof(type)*(count))

#endif
