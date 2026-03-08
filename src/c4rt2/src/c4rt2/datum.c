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
 * src/c4rt2/src/c4rt2/datum --
 *   
 */

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <dll-config.h>
#include <inttypes.h>

#include <c4rt2/api.h>
#include <c4rt2/datum.h>
#include <c4rt2/c4rt_package_versions.h>
#include <c4rt2/package.h>

#define UNREACHABLE(msg, ...) \
    do { \
        fprintf(stderr, msg, __VA_ARGS__); \
        abort(); \
    } while (0)

// 0b0'11111111111'0001'000000000000000000000000000000000000000000000000
const c4_datum_t gC4_Empty_Block = 0x7ff1000000000000;

// 0b1'00000000000'0000'000000000000000000000000000000000000000000000000
static const c4_datum_t remoteness_mask = 0x8000000000000000;
// 0b0'11111111111'0000'000000000000000000000000000000000000000000000000
static const c4_datum_t nan_mask = 0x7ff0000000000000;
// 0b0'00000000000'1111'000000000000000000000000000000000000000000000000
static const c4_datum_t type_mask = 0xf000000000000;
// 0b0'00000000000'0000'111111111111111111111111111111111111111111111111
static const c4_datum_t payload_mask = 0xffffffffffff;
// 0b0'00000000000'0000'111111111111111111111111111111111111111111111110
static const c4_datum_t pointer_mask = 0xfffffffffffe;

static void*
get_pointer_value(const c4_datum_t datum) {
    const c4_datum_t payload = datum & pointer_mask;
    return (void*)((int64_t)(payload << 16U) >> 16U);
}

static c4_datum_t
put_pointer_value(void* const ptr, const bool dynamic) {
    c4_datum_t ptr_payload = (uintptr_t)ptr & payload_mask;
    assert(ptr_payload == (uintptr_t)ptr
        && "Pointer contains more bits that 48. This is a fatal problem, because c4rt does bit fiddling.");

    assert((ptr_payload & 1) == 0
        && "Pointer is not aligned to at least 2 bytes. This is a fatal problem, because c4rt does bit fiddling.");
    ptr_payload |= (dynamic ? 1 : 0);

    return ptr_payload;
}

static c4_datum_t
shifted_type(const enum c4_datum_type type) {
    return (c4_datum_t)type << 48U;
}

void
c4rt_free(void* mem) {
    C4_FREE(mem);
}

C4RT_API bool
c4rt_datum_remoteness_of(const c4_datum_t datum) {
    return (bool)(datum & remoteness_mask);
}

C4RT_API int32_t
c4rt_datum_type_of(const c4_datum_t datum) {
    if ((datum & nan_mask) != nan_mask) return C4_Float;
    return (int32_t)((datum & type_mask) >> 48U);
}

c4_datum_t
c4rt_datum_from_static_ptr(const int32_t type, void* ptr) {
    return remoteness_mask | nan_mask | shifted_type((enum c4_datum_type)type) | put_pointer_value(ptr, false);
}

c4_datum_t
c4rt_datum_from_int32(int32_t i) {
    return nan_mask | shifted_type(C4_Integer) | (c4_datum_t)(uint32_t)i;
}

c4_datum_t
c4rt_datum_from_int64(const int64_t i) {
    int64_t* const buf = C4_NEW(int64_t, 1);
    *buf = i;
    return remoteness_mask | nan_mask | shifted_type(C4_Integer) | put_pointer_value(buf, true);
}

c4_datum_t
c4rt_datum_from_int(const int64_t i) {
    if (i >= (int64_t)(INT32_MIN) && i <= (int64_t)INT32_MAX)
        return c4rt_datum_from_int32((int32_t)i);
    return c4rt_datum_from_int64(i);
}

c4_datum_t
c4rt_datum_from_boolean(const int32_t i) {
    if (i == 0) return gC4_Empty_Block;
    return c4rt_datum_from_int32(1);
}

c4_datum_t
c4rt_datum_from_double(const double d) {
    return (c4_datum_t)d;
}

#if defined(_MSC_VER) && !defined(__clang__)
#  define ALIGNAS(x)
#else
#  define ALIGNAS(x) _Alignas(x)
#endif

static c4_datum_t
datum_from_sso_string(const char* const s, const size_t s_sz) {
    assert(s_sz < 6 && "larger than 5 characters cannot be sso optimized");
    c4_datum_t ret = nan_mask | shifted_type(C4_String);
    ALIGNAS(c4_datum_t) char buf[8] = {0};
    memcpy(buf, s, s_sz);
    ret |= *(c4_datum_t*)buf;
    return ret;
}

static c4_datum_t
datum_from_long_string(const char* const s, const size_t s_sz) {
    const c4_datum_t ret = remoteness_mask | nan_mask | shifted_type(C4_String);
    char* const full_ptr = C4_NEW(char, s_sz + 1 + sizeof(size_t));
    *(size_t*)full_ptr = s_sz;
    char* const data_ptr = full_ptr + sizeof(size_t);
    memcpy(data_ptr, s, s_sz + 1);
    return ret | put_pointer_value(data_ptr, true);
}

static c4_datum_t
datum_from_sized_string(const char* const s, const size_t s_sz) {
    if (s_sz <= 5) return datum_from_sso_string(s, s_sz);
    return datum_from_long_string(s, s_sz);
}

c4_datum_t
c4rt_datum_from_string(const char* const s) {
    assert(s && "s must not be null");
    const size_t s_sz = strlen(s);
    return datum_from_sized_string(s, s_sz);
}

c4_datum_t
c4rt_datum_from_string_sz(const char* s, const size_t s_sz) {
    return datum_from_sized_string(s, s_sz);
}

/**
 * The type used to represent a function that is stored inside a datum.
 * Behaves similarly to a package, but there are differences:
 *
 * - Does not handle effective nullary functions (i.e. non-closure zero
 *   parameter functions) as those are handled directly in the datum layer
 *   without even allocating this structure.
 * - Can not be "completed", as a datum's function is not a lazy wrapper
 *   around a function call, but a piece of data representing a computation.
 * - Could have "preloaded" arguments. Currently this is not really used, but
 *   basically currying.
 *
 * Based on this the following states are possible:
 *
 * 1. Holding non-closure:
 *      - \c context_sz_divided_bytes is zero.
 *      - \c fn_data is an array of package_size * base_arity long
 *        array of characters. The first package_size * preloaded_args_sz bytes
 *        are filled with the preloaded arguments, the other bytes are in an
 *        undefined state.
 *
 * 2. Holding closure:
 *      - \c context_sz_divided_bytes is some C > 0.
 *      - \c fn_data is an array of
 *          context_sz_divided_bytes * 16 + package_size * base_arity long
 *        array of characters. The first context_sz_divided_bytes * 16 are
 *        filled with the context's data (with possible padding.)
 *        The next package_size * preloaded_args_sz bytes are filled with the
 *        preloaded arguments, the other bytes are in an undefined state.
 */
struct datum_function {
    /// The callee function which may or may not be a closure, but has at least
    /// one effective parameter.
    c4rt_package_function_t* calc_fun;
    /// The number of normal (non-context) parameters of the callee function.
    uint16_t base_arity;
    /// The number of values already set for the function call. Always less than
    /// or equal to base_arity.
    uint16_t preloaded_args_sz;
    /// The size of the (first) context parameter to call to the function. If 0
    /// then callee is not a closure. Otherwise this value is taken by rounding
    /// up the actual context object's size to the next nearest (always >=)
    /// multiple of 16 and dividing it with 16. (For example. 1,8,16 get 1,
    /// 17 gets 2).
    uint16_t context_sz_divided_bytes;
    /// The ABI version of the held argument packages in fn_data.
    uint16_t package_version;
    /// Raw data.
    ALIGNAS(8) char fn_data[];
};

c4_datum_t
c4rt_datum_from_function(c4rt_package_function_t* const calc_fun,
                         const uint16_t base_arity,
                         const struct c4_package_t* const fn_data,
                         const size_t fn_data_sz) {
    assert(calc_fun && "calc_fun must be set");
    // nullary function optimization: skip allocation for a static sized ptr
    // (the calc_fun): if a block type is remote, but marked as static, it
    // points to a c4rt_package_function instead of a struct datum_function.
    // This allows not allocating stuff.
    if (base_arity == 0)
        return remoteness_mask | nan_mask | shifted_type(C4_Block)
               | put_pointer_value(calc_fun, false);

    // Note: this function does not handle closures
    uint16_t package_version = C4_PACKAGE_VERSION_1;
    if (fn_data) {
        assert(fn_data->version != 0 && "package version must be initialized");
        switch (fn_data->version) {
        case C4_PACKAGE_VERSION_1://
            package_version = C4_PACKAGE_VERSION_1;
            break;
        default: assert(false && "unsupported package version");
        }
    }
    const uint16_t package_size = c4rt_package_version_to_size(package_version);

    struct datum_function* const data = C4_MALLOC(
        sizeof(struct datum_function)       // header
        + 0u                                // context
        + base_arity * (size_t)package_size // args
    );

    data->calc_fun = calc_fun;
    data->base_arity = base_arity;
    data->context_sz_divided_bytes = 0;
    data->package_version = package_version;

    data->preloaded_args_sz = fn_data_sz;
    memcpy(data->fn_data, fn_data, fn_data_sz * (size_t)package_size);

    return remoteness_mask | nan_mask | shifted_type(C4_Block)
           | put_pointer_value(data, true);
}

static uint32_t
round_up(const uint32_t num, const size_t factor) {
    return num - 1u - (num - 1u) % factor + factor;
}

c4_datum_t
c4rt_datum_from_closure(c4rt_package_function_t* const calc_fun,
                        const uint16_t base_arity,
                        const void* const ctx,
                        const size_t ctx_sz,
                        const struct c4_package_t* const fn_data,
                        const size_t fn_data_sz) {
    assert(calc_fun && "calc_fun must be set");
    assert(ctx && "ctx must be set");
    assert(ctx_sz > 0 && "ctx_sz must be > 0");

    const uint16_t package_version = fn_data ? fn_data->version : C4_PACKAGE_VERSION_1;
    const uint16_t package_size = c4rt_package_version_to_size(package_version);

    const uint16_t ctx_sz_16multiple = round_up(ctx_sz, 16u);
    const uint16_t ctx_sz_divided_bytes = ctx_sz_16multiple / 16u;

    struct datum_function* const data = C4_MALLOC(
        sizeof(struct datum_function)       // header
        + ctx_sz_16multiple                 // context
        + base_arity * (size_t)package_size // args
    );

    data->calc_fun = calc_fun;
    data->base_arity = base_arity;
    data->context_sz_divided_bytes = ctx_sz_divided_bytes;
    data->package_version = package_version;

    const size_t ctx_offset = 0;
    const size_t args_offset = ctx_sz_16multiple;
    data->preloaded_args_sz = fn_data_sz;
    memcpy(data->fn_data + ctx_offset, ctx, ctx_sz);
    memcpy(data->fn_data + args_offset, fn_data, fn_data_sz * (size_t)package_size);

    return remoteness_mask | nan_mask | shifted_type(C4_Block)
           | put_pointer_value(data, true);
}

static void
datum_free_long_string(char* ptr) {
    C4_FREE(ptr - sizeof(size_t));
}

static bool
datum_is_ptr_dynamic(const c4_datum_t d) {
    return (bool)(d & 1);
}

void
c4rt_datum_free(const c4_datum_t d) {
    if (!c4rt_datum_remoteness_of(d)) return;
    if (!datum_is_ptr_dynamic(d)) return;
    if (c4rt_datum_type_of(d) == C4_String)
        return datum_free_long_string(get_pointer_value(d));

    void* ptr = get_pointer_value(d);
    C4_FREE(ptr);
}

static char*
datum_get_cstr_unck(c4_datum_t* const d) {
    return (char*)d; // - sizeof(c4_datum_t) + datum_local_data_offset;
}

static char*
datum_get_cstr_unck_remote(const c4_datum_t d) {
    char* pointer_value = get_pointer_value(d);
    return pointer_value;
}

static size_t
datum_get_cstr_size_unck_remote(const c4_datum_t d) {
    return *(size_t*)(
        (char*)get_pointer_value(d) - sizeof(size_t)
    );
}

static int32_t
datum_get_int32_unck(const c4_datum_t datum) {
    return (int32_t)(payload_mask & datum);
}

static int32_t
datum_get_int32_unck_remote(const c4_datum_t datum) {
    const int64_t* ptr = get_pointer_value(datum);
    return (int32_t)*ptr;
}

static int64_t
datum_get_int64_unck_remote(const c4_datum_t datum) {
    const int64_t* ptr = get_pointer_value(datum);
    return *ptr;
}

int32_t
c4rt_datum_get_int32(const c4_datum_t datum) {
    const enum c4_datum_type type = c4rt_datum_type_of(datum);
    const bool remoteness = c4rt_datum_remoteness_of(datum);
    assert(type == C4_Integer && "retrieving int32 from non-integer type is invalid, did you mean to coerce it?");

    if (!remoteness) return datum_get_int32_unck(datum);
    return datum_get_int32_unck_remote(datum);
}

int64_t
c4rt_datum_get_intó4(const c4_datum_t datum) {
    const enum c4_datum_type type = c4rt_datum_type_of(datum);
    const bool remoteness = c4rt_datum_remoteness_of(datum);
    assert(type == C4_Integer && "retrieving int64 from non-integer type is invalid, did you mean to coerce it?");

    if (!remoteness) return datum_get_int32_unck(datum);
    return datum_get_int64_unck_remote(datum);
}

double
c4rt_datum_get_double(const c4_datum_t datum) {
    const enum c4_datum_type type = c4rt_datum_type_of(datum);
    assert(type == C4_Float && "retrieving double from non-double type is invalid, did you mean to coerce it?");

    return *(double*)datum;
}

const char*
c4rt_datum_get_string(c4_datum_t* datum) {
    const enum c4_datum_type type = c4rt_datum_type_of(*datum);
    const bool remoteness = c4rt_datum_remoteness_of(*datum);
    assert(type == C4_String && "retrieving string from non-string type is invalid, did you mean to coerce it?");

    if (!remoteness) return datum_get_cstr_unck(datum);
    return datum_get_cstr_unck_remote(*datum);
}

static double
to_double(const char* buf) {
    double ret = 0.0;
    const int ret_sz = sscanf(buf, "%lf", &ret);
    if (ret_sz != 1) return 0.0;
    return ret;
}

static int64_t
to_int64(const char* buf) {
    int64_t ret = 0;
    const int ret_sz = sscanf(buf, "%"PRId64, &ret);
    if (ret_sz != 1) return 0;
    return ret;
}

static int32_t
to_int32(const char* buf) {
    int32_t ret = 0;
    const int ret_sz = sscanf(buf, "%"PRId32, &ret);
    if (ret_sz != 1) return 0;
    return ret;
}

int32_t
c4rt_datum_coerce_int32(c4_datum_t datum) {
    const enum c4_datum_type type = c4rt_datum_type_of(datum);
    const bool remoteness = c4rt_datum_remoteness_of(datum);

    switch (type) {
    case C4_Float: return (int32_t)*(double*)&datum;
    case C4_Block: return datum != gC4_Empty_Block;
    case C4_Integer: //
        if (!remoteness) return datum_get_int32_unck(datum);
        return datum_get_int32_unck_remote(datum);
    case C4_String: //
        if (!remoteness) return to_int32(datum_get_cstr_unck(&datum));
        return to_int32(datum_get_cstr_unck_remote(datum));
    }
    UNREACHABLE("invalid datum type %d", type);
}

int64_t
c4rt_datum_coerce_int64(c4_datum_t datum) {
    const enum c4_datum_type type = c4rt_datum_type_of(datum);
    const bool remoteness = c4rt_datum_remoteness_of(datum);

    switch (type) {
    case C4_Float: return (int64_t)*(double*)&datum;
    case C4_Block: return datum != gC4_Empty_Block;
    case C4_Integer://
        if (!remoteness) return datum_get_int32_unck(datum);
        return datum_get_int64_unck_remote(datum);
    case C4_String://
        if (!remoteness) return to_int64(datum_get_cstr_unck(&datum));
        return to_int64(datum_get_cstr_unck_remote(datum));
    }
    UNREACHABLE("invalid datum type %d", type);
}

double
c4rt_datum_coerce_double(c4_datum_t datum) {
    const enum c4_datum_type type = c4rt_datum_type_of(datum);
    const bool remoteness = c4rt_datum_remoteness_of(datum);

    switch (type) {
    case C4_Float: return *(double*)&datum;
    case C4_Block: return datum != gC4_Empty_Block;
    case C4_Integer://
        if (!remoteness) return datum_get_int32_unck(datum);
        return (double)datum_get_int64_unck_remote(datum);
    case C4_String://
        if (!remoteness) return to_double(datum_get_cstr_unck(&datum));
        return to_double(datum_get_cstr_unck_remote(datum));
    }
    UNREACHABLE("invalid datum type %d", type);
}

static char*
from_double(const double d) {
    char* const buf = C4_NEW(char, 64);
    snprintf(buf, 64u, "%lf", d);
    return buf;
}

static char*
from_int64(const int64_t i) {
    char* const buf = C4_NEW(char, 22);
    snprintf(buf, 22u, "%"PRId64, i);
    return buf;
}

static char*
from_int64x(const int64_t i) {
    char* const buf = C4_NEW(char, 22);
    snprintf(buf, 22u, "(%#"PRIx64")", i);
    return buf;
}

static char*
from_int32(const int32_t i) {
    char* const buf = C4_NEW(char, 12);
    snprintf(buf, 12u, "%"PRId32, i);
    return buf;
}

C4RT_API c4_datum_t
_Cs7println1E(struct c4_package_t* x) {
    const c4_datum_t datum = c4rt_package_evaluate(x);
    char* const str = c4rt_datum_coerce_string(datum);
    printf("%s\n", str);
    C4_FREE(str);
    return datum;
}

C4RT_API c4_datum_t
_Cs5print1E(struct c4_package_t* x) {
    const c4_datum_t datum = c4rt_package_evaluate(x);
    char* const str = c4rt_datum_coerce_string(datum);
    printf("%s", str);
    C4_FREE(str);
    return datum;
}

C4RT_API c4_datum_t
_Cs9str_empty1E(struct c4_package_t* x) {
    const c4_datum_t datum = c4rt_package_evaluate(x);
    char* const str = c4rt_datum_coerce_string(datum);
    const bool empty = str[0] == '\0';
    const c4_datum_t ret = c4rt_datum_from_boolean(empty);
    C4_FREE(str);
    return ret;
}

C4RT_API c4_datum_t
_Cs9blk_empty1E(struct c4_package_t* x) {
    const c4_datum_t datum = c4rt_package_evaluate(x);
    return c4rt_datum_from_boolean(datum == gC4_Empty_Block);
}

C4RT_API c4_datum_t
_Cs9nil_block0E() {
    return gC4_Empty_Block;
}

C4RT_API c4_datum_t
_Cs3int1E(struct c4_package_t* x) {
    const c4_datum_t datum = c4rt_package_evaluate(x);
    const int64_t i = c4rt_datum_coerce_int64(datum);
    return c4rt_datum_from_int(i);
}

C4RT_API c4_datum_t
_Co2EE2E(struct c4_package_t* a, struct c4_package_t* b) {
    const c4_datum_t a_val = c4rt_package_evaluate(a);
    const c4_datum_t b_val = c4rt_package_evaluate(b);
    const c4_datum_t ret = c4rt_datum_eq(a_val, b_val);
    c4rt_datum_free(a_val);
    c4rt_datum_free(b_val);
    return ret;
}

C4RT_API c4_datum_t
_Co1m2E(struct c4_package_t* a, struct c4_package_t* b) {
    const c4_datum_t a_val = c4rt_package_evaluate(a);
    const c4_datum_t b_val = c4rt_package_evaluate(b);
    const int64_t a_i = c4rt_datum_coerce_int64(a_val);
    const int64_t b_i = c4rt_datum_coerce_int64(b_val);
    c4rt_datum_free(a_val);
    c4rt_datum_free(b_val);
    return c4rt_datum_from_int(a_i - b_i);
}

C4RT_API c4_datum_t
_Co1p2E(struct c4_package_t* a, struct c4_package_t* b) {
    const c4_datum_t a_val = c4rt_package_evaluate(a);
    const c4_datum_t b_val = c4rt_package_evaluate(b);
    const int64_t a_i = c4rt_datum_coerce_int64(a_val);
    const int64_t b_i = c4rt_datum_coerce_int64(b_val);
    c4rt_datum_free(a_val);
    c4rt_datum_free(b_val);
    return c4rt_datum_from_int(a_i + b_i);
}

C4RT_API c4_datum_t
_Co1e1E(struct c4_package_t* a) {
    const c4_datum_t a_val = c4rt_package_evaluate(a);
    const bool bool_val = a_val != gC4_Empty_Block;
    return c4rt_datum_from_boolean(!bool_val);
}

C4RT_API c4_datum_t
_Cs3cat2E(struct c4_package_t* a, struct c4_package_t* b) {
    const c4_datum_t a_val = c4rt_package_evaluate(a);
    const c4_datum_t b_val = c4rt_package_evaluate(b);
    char* a_str = c4rt_datum_coerce_string(a_val);
    char* b_str = c4rt_datum_coerce_string(b_val);
    const size_t a_str_sz = strlen(a_str);
    const size_t b_str_sz = strlen(b_str);
    const size_t total_sz = a_str_sz + b_str_sz + 1;
    char* const buf = C4_NEW(char, total_sz);
    memcpy(buf, a_str, a_str_sz);
    memcpy(buf + a_str_sz, b_str, b_str_sz);
    buf[total_sz - 1] = '\0';
    const c4_datum_t ret = c4rt_datum_from_string(buf);
    C4_FREE(buf);
    C4_FREE(a_str);
    C4_FREE(b_str);
    c4rt_datum_free(a_val);
    c4rt_datum_free(b_val);
    return ret;
}

C4RT_API c4_datum_t
_Cs6readln0E() {
    const int buf_sz = 1024;
    char* const buf = C4_NEW(char, buf_sz);

    if (fgets(buf, buf_sz, stdin) != NULL)
        buf[strcspn(buf, "\n")] = '\0';

    const c4_datum_t ret = c4rt_datum_from_string(buf);
    C4_FREE(buf);
    return ret;
}

C4RT_API c4_datum_t
_Cs2if3E(struct c4_package_t* cond, struct c4_package_t* t, struct c4_package_t* f) {
    const c4_datum_t cond_val = c4rt_package_evaluate(cond);
    if (cond_val == gC4_Empty_Block) {
        const c4_datum_t f_code = c4rt_package_evaluate(f);
        const c4_datum_t ret = c4rt_datum_evaluate(f_code, NULL);
        c4rt_datum_free(cond_val);
        return ret;
    }
    const c4_datum_t t_code = c4rt_package_evaluate(t);
    const c4_datum_t ret = c4rt_datum_evaluate(t_code, NULL);
    c4rt_datum_free(cond_val);
    return ret;
}

char*
c4rt_datum_coerce_string(c4_datum_t datum) {
    const enum c4_datum_type type = c4rt_datum_type_of(datum);
    const bool remoteness = c4rt_datum_remoteness_of(datum);

    switch (type) {
    case C4_Float: return from_double(*(double*)&datum);
    case C4_Block: //
        if (datum != gC4_Empty_Block)
            return from_int64x((int64_t)(uintptr_t)get_pointer_value(datum));
        return C4_STRDUP("{}");
    case C4_Integer: //
        if (!remoteness) return from_int32(datum_get_int32_unck(datum));
        return from_int64(datum_get_int64_unck_remote(datum));
    case C4_String: //
        if (!remoteness) return C4_STRDUP(datum_get_cstr_unck(&datum));
        return C4_STRDUP(datum_get_cstr_unck_remote(datum));
    }
    UNREACHABLE("invalid datum type %d", type);
}

c4_datum_t
c4rt_datum_dup(const c4_datum_t datum) {
    // since non-remote data is always fully encapsulated inside the 64-bit
    // block, the easiest way to copy it is just copy the int64 type
    if (!c4rt_datum_remoteness_of(datum)) return datum;

    const enum c4_datum_type type = c4rt_datum_type_of(datum);
    switch (type) {
    case C4_Float:
    // this can only happen in edge cases where a -NaN occurs. Nevertheless,
    // the solution is trivial:
    // FALLTHROUGH
    case C4_Block: return datum;
    case C4_Integer: return c4rt_datum_from_int64(c4rt_datum_get_intó4(datum));
    case C4_String:
        // todo: ref-counted CoW strings
        return datum_from_sized_string(
            datum_get_cstr_unck_remote(datum),
            datum_get_cstr_size_unck_remote(datum));
    }
    UNREACHABLE("invalid datum type %d", type);
}

c4_datum_t
c4rt_datum_eq(c4_datum_t a, c4_datum_t b) {
    const enum c4_datum_type a_type = c4rt_datum_type_of(a);
    const enum c4_datum_type b_type = c4rt_datum_type_of(b);
    if (a_type != b_type) return gC4_Empty_Block;

    switch (a_type) {
    case C4_Float: return c4rt_datum_from_boolean(a == b);
    case C4_Block://
        if (a == gC4_Empty_Block) return c4rt_datum_from_boolean(b == gC4_Empty_Block);
        return gC4_Empty_Block;
    case C4_Integer: {
        const int64_t a_64 = c4rt_datum_get_intó4(a);
        const int64_t b_64 = c4rt_datum_get_intó4(b);
        return c4rt_datum_from_boolean(a_64 == b_64);
    }
    case C4_String: {
        const char* const a_str = c4rt_datum_get_string(&a);
        const char* const b_str = c4rt_datum_get_string(&b);
        return c4rt_datum_from_boolean(strcmp(a_str, b_str) == 0);
    }
    }
    UNREACHABLE("invalid datum type %d", a_type);
}

#include "dynamic_call_hacks.h"

c4_datum_t
c4rt_datum_preload_arguments(const c4_datum_t datum,
                             const struct c4_package_t** const args,
                             const size_t args_sz) {
    assert((args_sz == 0 || args) && "args must be null or valid");
    if (args_sz == 0) return datum;

    const enum c4_datum_type type = c4rt_datum_type_of(datum);
    assert(type == C4_Block && "preloading arguments only supported for blocks");
    if (datum == gC4_Empty_Block) return gC4_Empty_Block;

    assert(datum_is_ptr_dynamic(datum) &&
        "preloading non-null argument set to nullary datum object");

    const struct datum_function* const fn_data_old = get_pointer_value(datum);
    const size_t fn_data_sz = (size_t)fn_data_old->context_sz_divided_bytes * 16u
                              + (size_t)fn_data_old->base_arity * sizeof(struct c4_package_t*);
    struct datum_function* const fn_data = C4_ALLOCATE(sizeof(struct datum_function) + fn_data_sz);
    memcpy(fn_data, fn_data_old, sizeof(struct datum_function) + fn_data_sz);

    const size_t args_offset = (size_t)fn_data->context_sz_divided_bytes * 16u;
    char* const callee_args = fn_data->fn_data + args_offset;

    const size_t missing_argument_sz = fn_data->base_arity - fn_data->preloaded_args_sz;
    assert(missing_argument_sz >= args_sz && "too many arguments to preload");
    assert((args && *args) && "missing arguments require to be passed");
    // above assert is always required because we early return in the second line
    // thusly args_sz > 0 by here, so 1) there is no early return check for
    // missing_argument_sz == 0 here, since missing_argument_sz >= args_sz > 0.
    // and 2) args_sz > 0 means at least one object must be present in args, so
    // (args && *args) must be true, hence the assert.

    struct c4_package_t* const missing_arguments = (struct c4_package_t*)callee_args + fn_data->preloaded_args_sz;
    memcpy(missing_arguments, args, args_sz * sizeof(struct c4_package_t*));
    fn_data->preloaded_args_sz += args_sz;

    assert(fn_data->base_arity >= fn_data->preloaded_args_sz && "block datum invariant broken");
    return remoteness_mask | nan_mask | shifted_type(C4_Block)
           | put_pointer_value(fn_data, true);
}

c4_datum_t
c4rt_datum_evaluate(const c4_datum_t datum,
                    struct c4_package_t** const args) {
    const enum c4_datum_type type = c4rt_datum_type_of(datum);
    if (type != C4_Block) return datum;
    if (datum == gC4_Empty_Block) return gC4_Empty_Block;

    void* ptr = get_pointer_value(datum);
    if (!datum_is_ptr_dynamic(datum)) return ((c4rt_package_function_t*)ptr)();

    struct datum_function* fn_data = ptr;

    const size_t args_offset = (size_t)fn_data->context_sz_divided_bytes * 16u;
    char* const callee_args = fn_data->fn_data + args_offset;

    const size_t missing_argument_sz = fn_data->base_arity - fn_data->preloaded_args_sz;
    if (missing_argument_sz) {
        // fprintf(stderr, "missing: %zu, passed: %p (->%p)\n", missing_argument_sz, args, args ? *args : NULL);
        assert((args && *args) && "missing arguments require to be passed");

        struct c4_package_t* const missing_arguments = (struct c4_package_t*)callee_args + fn_data->preloaded_args_sz;
        memcpy(missing_arguments, args, missing_argument_sz * sizeof(struct c4_package_t*));
    }

    void* const context = fn_data->context_sz_divided_bytes
                          ? fn_data->fn_data
                          : NULL;

    // if args are empty it does not matter which version we use as there are
    // no packages involved in the call
    const uint16_t version = (args && *args) ? (*args)->version : C4_PACKAGE_VERSION_1;

    switch (version) {
    case C4_PACKAGE_VERSION_1:
    default://
        if (context) {
            return c4_dynamic_call_v1_ctx(fn_data->base_arity,
                                          fn_data->calc_fun,
                                          context,
                                          (void*)callee_args);
        }
        return c4_dynamic_call_v1(fn_data->base_arity,
                                  fn_data->calc_fun,
                                  (void*)callee_args);
    }
    assert(false);
}
