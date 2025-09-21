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
 * Originally created: 2025-08-08.
 *
 * src/c4rt2/include/c4rt2/package --
 *   Provides the C4 runtime's ABI stable package handling symbols.
 */
#ifndef BLAST_C4RT_PACKAGE_H
#define BLAST_C4RT_PACKAGE_H

#include <c4rt2/api.h>
#include <c4rt2/package_type.h>

/// c4rt_package_init_from_function(*pkg, *calc_fun, *fn_data, fn_data_sz) --
///     Initializes a new C4 package at pkg. There must be enough space in the
///     buffer for package_version bytes which must be set on the object.
///     The correct package version will be initialized based on this field.
///
///     If fn_data is null, fn_data_sz must be zero.
///     If not null, fn_data must point to an array of size fn_data_sz (in bytes)
///     that contains packages of the same version as specified in pkg's
///     package_version.
///
///     Behavior is undefined if either pkg or calc_fun is null.
C4RT_API void
c4rt_package_init_from_function(struct c4_package_t* pkg,
                                c4rt_package_function_t* calc_fun,
                                const struct c4_package_t** fn_data,
                                uint32_t fn_data_sz);

/// c4rt_package_init_from_closure(*pkg, *calc_fun,
///                                *ctx, ctx_sz,
///                                *fn_data, fn_data_sz) --
///     Initializes a new C4 package at pkg. There must be enough space in the
///     buffer for package_version bytes which must be set on the object.
///     The correct package version will be initialized based on this field.
///
///     If fn_data is null, fn_data_sz must be zero.
///     If not null, fn_data must point to an array of size fn_data_sz (in bytes)
///     that contains packages of the same version as specified in pkg's
///     package_version.
///
///     Context passed in [ctx, ctx_sz|) bytes can be any structure and will
///     be passed as the first argument to the given function. Note that it
///     is not deep copied nor released: only the given bytes are copied over
///     and passed to the function.
///
///     Behavior is undefined if either ctx, pkg, or calc_fun is null.
C4RT_API void
c4rt_package_init_from_closure(struct c4_package_t* pkg,
                               c4rt_package_function_t* calc_fun,
                               const void* ctx,
                               uint32_t ctx_sz,
                               const struct c4_package_t** fn_data,
                               uint32_t fn_data_sz);

/// c4rt_package_init_from_dynamic(*pkg, datum, *fn_data, fn_data_sz) --
///     Initializes a new C4 package at pkg. The callee entity is a valid datum
///     object that has exactly fn_data_sz amount of space for arguments. When
///     the package gets evaluated, the datum object is evaluated with the
///     arguments preloaded at construction (now).
///
///     If fn_data is null, fn_data_sz must be zero.
///
///     Behavior is undefined if either pkg is null.
C4RT_API void
c4rt_package_init_from_dynamic(struct c4_package_t* pkg,
                               c4_datum_t datum,
                               const struct c4_package_t** fn_data,
                               uint32_t fn_data_sz);

/// c4rt_package_set_from_result(*pkg, datum) --
///     Packages a given datum as a successfully calculated result into pkg.
///
///     Behavior is undefined if pkg is null.
C4RT_API void
c4rt_package_init_from_result(struct c4_package_t* pkg,
                              c4_datum_t datum);

/// c4rt_package_evaluate(pkg) --
///     Calculates the value of pkg if it has not yet been calculated and
///     returns the result. Modifies pkg to store the calculated datum, and
///     pkg will thereafter report as completed.
///     If pkg is already completed, just returns the stored result.
///
///     Behavior is undefined if pkg is null.
C4RT_API c4_datum_t
c4rt_package_evaluate(struct c4_package_t* pkg);

#endif
