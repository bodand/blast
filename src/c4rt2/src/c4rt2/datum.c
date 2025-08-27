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

#include <c4rt2/datum.h>
#include <dll-config.h>

#define UNREACHABLE(msg, ...) \
    do { \
        fprintf(stderr, msg, __VA_ARGS__); \
        abort(); \
    } while (0)

// 0b0'11111111111'0001'000000000000000000000000000000000000000000000000
const c4_datum_t gC4_Empty_Block = 0x7ff1000000000000;

// 0b1'00000000000'0000'000000000000000000000000000000000000000000000000
const static c4_datum_t remoteness_mask = 0x8000000000000000;
// 0b0'11111111111'0000'000000000000000000000000000000000000000000000000
const static c4_datum_t nan_mask = 0x7ff0000000000000;
// 0b0'00000000000'1111'000000000000000000000000000000000000000000000000
const static c4_datum_t type_mask = 0xf000000000000;
// 0b0'00000000000'0000'111111111111111111111111111111111111111111111111
const static c4_datum_t payload_mask = 0xffffffffffff;
// 0b0'00000000000'0000'111111111111111111111111111111111111111111111110
const static c4_datum_t pointer_mask = 0xfffffffffffe;
//   1'234567890123'456
const static size_t datum_local_data_offset = 16 / CHAR_BIT;

static void*
get_pointer_value(const c4_datum_t datum) {
    const c4_datum_t payload = datum & pointer_mask;
    return (void*)((int64_t)(payload << 16U) >> 16U);
}

static c4_datum_t
put_pointer_value(void* const ptr, bool dynamic) {
    c4_datum_t ptr_payload = (uintptr_t)ptr & payload_mask;
    assert(ptr_payload == (uintptr_t)ptr
        && "Pointer contains more bits that 48. This is a fatal problem, because c4rt does bit fiddling.");

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
c4rt_datum_from_int32(const int32_t i) {
    return nan_mask | shifted_type(C4_Integer) | (c4_datum_t)i;
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
c4rt_datum_from_double(const double d) {
    return (c4_datum_t)d;
}

static c4_datum_t
datum_from_sso_string(const char* const s, const size_t s_sz) {
    assert(s_sz < 6 && "larger than 5 characters cannot be sso optimized");
    const c4_datum_t ret = nan_mask | shifted_type(C4_String);
    _Alignas(c4_datum_t) char buf[8];
    memcpy(buf, s, s_sz);
    return ret | *(c4_datum_t*)buf;
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
    const size_t s_sz = strlen(s);
    return datum_from_sized_string(s, s_sz);
}

c4_datum_t
c4rt_datum_from_string_sz(const char* s, const size_t s_sz) {
    return datum_from_sized_string(s, s_sz);
}

struct datum_function {
    c4rt_package_function_t* calc_fun;
    uint16_t effective_arity;
    uint16_t preloaded_args_sz;
    uint32_t reserved_0;
    struct c4_package_t fn_data[];
};

c4_datum_t
c4rt_datum_from_function(c4rt_package_function_t* const calc_fun,
                         const uint16_t eff_arity,
                         const struct c4_package_t* const fn_data,
                         const size_t fn_data_sz) {
    assert(calc_fun && "calc_fun must be set");
    // nullary function optimization: skip allocation for a static sized ptr
    // (the calc_fun): if a block type is remote, but marked as static, it
    // points to a c4rt_package_function instead of a struct datum_function.
    // This allows not allocating stuff.
    if (eff_arity == 0)
        return remoteness_mask | nan_mask | shifted_type(C4_Block)
               | put_pointer_value(calc_fun, false);

    struct datum_function* const data = C4_MALLOC(sizeof(struct datum_function)
        + eff_arity * sizeof(struct c4_package_t));
    data->calc_fun = calc_fun;
    data->effective_arity = eff_arity;
    data->preloaded_args_sz = fn_data_sz;
    memcpy(data->fn_data, fn_data, fn_data_sz * sizeof(struct c4_package_t));
    memset(data->fn_data + fn_data_sz,
           0,
           (eff_arity - fn_data_sz) * sizeof(struct c4_package_t));

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
    return get_pointer_value(d);
}

static size_t
datum_get_cstr_size_unck_remote(const c4_datum_t d) {
    return *(size_t*)(
        (char*)get_pointer_value(d) - sizeof(size_t)
    );
}

static int32_t
datum_get_int32_unck(const c4_datum_t datum) {
    return ((int32_t)(payload_mask & datum));
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
    const int ret_sz = sscanf(buf, "%lld", &ret);
    if (ret_sz != 1) return 0;
    return ret;
}

static int32_t
to_int32(const char* buf) {
    int32_t ret = 0;
    const int ret_sz = sscanf(buf, "%d", &ret);
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
        return ((double)(datum_get_int64_unck_remote(datum)));
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
    snprintf(buf, 22u, "%lld", i);
    return buf;
}

static char*
from_int32(const int32_t i) {
    char* const buf = C4_NEW(char, 12);
    snprintf(buf, 12u, "%lld", (long long)i);
    return buf;
}

char*
c4rt_datum_coerce_string(c4_datum_t datum) {
    const enum c4_datum_type type = c4rt_datum_type_of(datum);
    const bool remoteness = c4rt_datum_remoteness_of(datum);

    switch (type) {
    case C4_Float: return from_double(*(double*)&datum);
    case C4_Block: //
        if (datum != gC4_Empty_Block)
            return from_int64((int64_t)(uintptr_t)get_pointer_value(datum));
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

#include "dynamic_call_hacks.h"

c4_datum_t
c4rt_datum_evaluate(const c4_datum_t datum,
                    struct c4_package_t* const args) {
    const enum c4_datum_type type = c4rt_datum_type_of(datum);
    if (type != C4_Block)
        return datum;
    if (datum == gC4_Empty_Block) return gC4_Empty_Block;

    void* ptr = get_pointer_value(datum);
    if (!datum_is_ptr_dynamic(datum)) {
        return ((c4rt_package_function_t*)ptr)();
    }

    struct datum_function* fn_data = ptr;
    struct c4_package_t* const arguments_array = fn_data->fn_data + fn_data->preloaded_args_sz;
    memcpy(arguments_array, args,
           (fn_data->effective_arity - fn_data->preloaded_args_sz) * sizeof(struct c4_package_t));

    return c4_dynamic_call(fn_data->effective_arity,
                           fn_data->calc_fun,
                           fn_data->fn_data);
}
