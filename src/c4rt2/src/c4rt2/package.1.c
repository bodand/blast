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
 * src/c4rt2/src/c4rt2/c4rt_package.1 --
 *   Implementation of the version 1 package system.
 */

#include <assert.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <c4rt2/package.h>
#include <c4rt2/datum.h>
#include <c4rt2/datum_type.h>
#include <c4rt2/c4rt_package_versions.h>

#include "package.1.h"

#include <dll-config.h>

#define tail [[clang::musttail]] return

#define max(a, b) (((a) > (b)) ? (a) : (b))

static c4_ptr64_t
pad_pointer_1(void* const ptr) {
    return ((uintptr_t)ptr) << (CHAR_BIT * (sizeof(c4_ptr64_t) - sizeof(void*)));
}

static void*
unpad_pointer_1(const c4_ptr64_t ptr) {
    return (void*)(ptr >> (CHAR_BIT * (sizeof(c4_ptr64_t) - sizeof(void*))));
}

void
c4rt_package_init_from_function_v1(struct c4_package_v1_t* const pkg,
                                   c4rt_package_function_t* const calc_fun,
                                   const struct c4_package_v1_t** fn_data,
                                   const uint16_t fn_data_sz_bytes) {
    assert(pkg && "pkg must not be null");
    assert(calc_fun && "calc_fun must not be null");
    assert((fn_data_sz_bytes == 0 || fn_data) && "fn_data must be null or valid");
    assert(pkg->version == C4_PACKAGE_VERSION_1);
    pkg->completed = false;
    pkg->dynamic_datum = false;
    pkg->function_arity = fn_data_sz_bytes / sizeof(struct c4_package_v1_t*);

    // Note: this function does not handle closures.
    pkg->context_sz_divided_bytes = 0;

    // nullary functions don't need an allocation for their parameters, just
    // embed the function pointer as the data
    if (fn_data_sz_bytes == 0) {
        pkg->data = pad_pointer_1(calc_fun);
        return;
    }

    struct c4_package_v1_function_payload* payload =
            C4_ALLOCATE(
                sizeof(struct c4_package_v1_function_payload) +
                fn_data_sz_bytes
            );
    assert(payload && "alloc failed");
    payload->calc_fun = calc_fun;
    memcpy(payload->args_untyped, (const void*)fn_data, fn_data_sz_bytes);

    pkg->data = pad_pointer_1(payload);
}

static uint32_t
round_up(const uint32_t num, const size_t factor) {
    return num - 1u - (num - 1u) % factor + factor;
}

void
c4rt_package_init_from_closure_v1(struct c4_package_v1_t* const pkg,
                                  c4rt_package_function_t* const calc_fun,
                                  const void* const ctx,
                                  const uint32_t ctx_sz_bytes,
                                  const struct c4_package_v1_t** const fn_data,
                                  const uint32_t fn_data_sz_bytes) {
    assert(pkg && "pkg must not be null");
    assert(calc_fun && "calc_fun must not be null");
    assert(ctx && "ctx must not be null");
    assert(ctx_sz_bytes > 0 && "ctx_sz_bytes must be > 0");
    assert((fn_data_sz_bytes == 0 || fn_data) && "fn_data must be null or valid");
    assert(pkg->version == C4_PACKAGE_VERSION_1);
    pkg->completed = false;
    pkg->dynamic_datum = false;

    const size_t pkg_sz = sizeof(struct c4_package_v1_t*);
    pkg->function_arity = fn_data_sz_bytes / pkg_sz + 1u;

    const uint32_t ctx_size_as_pkg_multiple = round_up(ctx_sz_bytes, pkg_sz);
    const uint32_t ctx_size_as_pkg_multiple_divided_bytes =
            ctx_size_as_pkg_multiple / pkg_sz;
    assert(
        ctx_size_as_pkg_multiple_divided_bytes < UINT16_MAX
        && "context size too large for package version 1"
    );
    pkg->context_sz_divided_bytes = (uint16_t)ctx_size_as_pkg_multiple_divided_bytes;

    const uint32_t ctx_offset = 0u;
    const uint32_t proper_args_offset = max(_Alignof(struct c4_package_v1_t*), ctx_size_as_pkg_multiple);
    const uint32_t args_padding = ctx_size_as_pkg_multiple - proper_args_offset;
    assert(
        proper_args_offset >= ctx_offset + ctx_sz_bytes
        && "internal error: args offset overlaps context"
    );

    struct c4_package_v1_function_payload* payload =
            C4_ALLOCATE(
                sizeof(struct c4_package_v1_function_payload) + // header
                ctx_size_as_pkg_multiple + // context
                args_padding +
                fn_data_sz_bytes
            ); // args
    assert(payload && "alloc failed");
    payload->calc_fun = calc_fun;
    memcpy(payload->args_untyped + ctx_offset, ctx, ctx_sz_bytes);
    memcpy(payload->args_untyped + proper_args_offset, (const void*)fn_data, fn_data_sz_bytes);

    pkg->data = pad_pointer_1(payload);
}

void
c4rt_package_init_from_dynamic_v1(struct c4_package_v1_t* pkg,
                                  c4_datum_t datum,
                                  const struct c4_package_v1_t** fn_data,
                                  const uint32_t fn_data_sz_bytes) {
    assert(pkg && "pkg must not be null");
    assert(pkg->version == C4_PACKAGE_VERSION_1);
    assert((fn_data_sz_bytes == 0 || fn_data) && "fn_data must be null or valid");
    pkg->completed = false;
    pkg->dynamic_datum = true;
    pkg->function_arity = fn_data_sz_bytes / sizeof(struct c4_package_v1_t*);

    // Note: this function does not handle closures.
    pkg->context_sz_divided_bytes = 0;

    // We can save an allocation if we embed the new parameters into the
    // existing datum object's memory area which already has enough memory.
    // Zero sized preloads are a nop, no need to pre-check it.
    datum = c4rt_datum_preload_arguments(
        datum,
        (const struct c4_package_t**)fn_data,
        fn_data_sz_bytes / sizeof(struct c4_package_v1_t*)
    );
    pkg->data = datum;
}

void
c4rt_package_init_from_result_v1(struct c4_package_v1_t* const pkg,
                                 const c4_datum_t datum) {
    assert(pkg && "pkg must not be null");
    assert(pkg->version == C4_PACKAGE_VERSION_1);
    pkg->completed = true;
    pkg->data = datum;
}

#include "dynamic_call_hacks.h"

static c4_datum_t
package1_set_datum(struct c4_package_v1_t* const pkg, const c4_datum_t datum) {
    pkg->completed = true;
    pkg->data = datum;
    return datum;
}

static c4_datum_t
package1_eval_dynamic_datum(void* pkg_raw) {
    struct c4_package_v1_t* const pkg = pkg_raw;
    const c4_datum_t datum = c4rt_datum_evaluate(pkg->data, NULL);
    return package1_set_datum(pkg, datum);
}

static c4_datum_t
package1_eval_raw_funptr(void* pkg_raw) {
    struct c4_package_v1_t* const pkg = pkg_raw;
    c4rt_package_function_t* const calc_fun = unpad_pointer_1(pkg->data);
    assert(calc_fun && "invalid pointer while evaluating packager as raw function");

    const c4_datum_t datum = calc_fun();
    return package1_set_datum(pkg, datum);
}

static c4_datum_t
package1_eval_function(void* pkg_raw) {
    struct c4_package_v1_t* const pkg = pkg_raw;
    struct c4_package_v1_function_payload* const payload = unpad_pointer_1(pkg->data);
    assert(payload && "invalid pointer while evaluating package as function");

    const c4_datum_t datum = c4_dynamic_call_v1(pkg);
    C4_FREE(payload);
    return package1_set_datum(pkg, datum);
}

static c4_datum_t
package1_eval_closure(void* pkg_raw) {
    struct c4_package_v1_t* const pkg = pkg_raw;
    struct c4_package_v1_function_payload* const payload = unpad_pointer_1(pkg->data);
    assert(payload && "invalid pointer while evaluating package as closure");

    const c4_datum_t datum = c4_dynamic_call_v1_ctx(pkg);
    C4_FREE(payload);
    return package1_set_datum(pkg, datum);
}

static c4_datum_t
package1_eval_structured_ptr(void* pkg_raw) {
    struct c4_package_v1_t* const pkg = pkg_raw;

    if (pkg->context_sz_divided_bytes == 0)
        tail package1_eval_function(pkg);

    tail package1_eval_closure(pkg);
}

static c4_datum_t
package1_eval_pointer(void* pkg_raw) {
    struct c4_package_v1_t* const pkg = pkg_raw;

    if (pkg->function_arity == 0)
        tail package1_eval_raw_funptr(pkg);

    tail package1_eval_structured_ptr(pkg);
}

c4_datum_t
c4rt_package_evaluate_v1(void* pkg_raw) {
    struct c4_package_v1_t* const pkg = pkg_raw;
    assert(pkg && "pkg must not be null");
    assert(pkg->version == C4_PACKAGE_VERSION_1);
    if (pkg->completed) return pkg->data;

    // arity/context does not matter for dynamic_datum, as the actual
    // data is encoded into the datum object. When evaluating, we just need
    // to call datum_evaluate and hope that it has been properly preloaded
    // with the correct arguments. (Otherwise the runtime crashes.)
    if (pkg->dynamic_datum)
        tail package1_eval_dynamic_datum(pkg);

    tail package1_eval_pointer(pkg);
}

void
c4rt_package_free_v1(const struct c4_package_v1_t* const pkg) {
    assert(pkg && "pkg must not be null");
    assert(pkg->version == C4_PACKAGE_VERSION_1);
    if (pkg->completed) return;
    if (pkg->function_arity == 0) return;

    void* ptr = unpad_pointer_1(pkg->data);
    C4_FREE(ptr);
}
