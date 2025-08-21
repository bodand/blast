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

#include <cassert>
#include <iostream>
#include <charconv>
#include <bit>
#include <format>
#include <print>

#include <c4rt2/datum.h>
#include <dll-config.h>

#define UNREACHABLE(msg, ...) \
    do { \
        std::println(std::cerr, msg, __VA_ARGS__); \
        abort(); \
    } while (0)

const c4_datum_t gC4_Empty_Block = 0b0'11111111111'0001'000000000000000000000000000000000000000000000000;

static_assert(alignof(void*) > 2);

namespace {
    constexpr auto remoteness_mask = 0b1'00000000000'0000'000000000000000000000000000000000000000000000000;
    constexpr auto nan_mask = 0b0'11111111111'0000'000000000000000000000000000000000000000000000000;
    constexpr auto type_mask = 0b0'00000000000'1111'000000000000000000000000000000000000000000000000;
    constexpr auto payload_mask = 0b0'00000000000'0000'111111111111111111111111111111111111111111111111;
    constexpr auto pointer_mask = 0b0'00000000000'0000'111111111111111111111111111111111111111111111110;

    void*
    get_pointer_value(const c4_datum_t datum) {
        const auto payload = datum & pointer_mask;
        return std::bit_cast<void*>(static_cast<std::int64_t>(payload << 16U) >> 16U);
    }

    c4_datum_t
    put_pointer_value(void* const ptr, bool dynamic) {
        auto ptr_payload = (std::bit_cast<std::uintptr_t>(ptr) & payload_mask);
        assert(ptr_payload == std::bit_cast<std::uintptr_t>(ptr)
            && "Pointer contains more bits that 48. This is a fatal problem, because c4rt does bit fiddling.");

        ptr_payload |= (dynamic ? 1 : 0);

        return ptr_payload;
    }

    constexpr c4_datum_t
    shifted_type(const c4_datum_type type) {
        return static_cast<c4_datum_t>(type) << 48U;
    }
}

void
c4rt_free(void* mem) {
    C4_FREE(mem);
}

C4RT_API bool
c4rt_datum_remoteness_of(const c4_datum_t datum) {
    return static_cast<bool>(datum & remoteness_mask);
}

C4RT_API int32_t
c4rt_datum_type_of(const c4_datum_t datum) {
    if ((datum & nan_mask) != nan_mask) return C4_Float;
    return static_cast<int32_t>((datum & type_mask) >> 48U);
}

c4_datum_t
c4rt_datum_from_static_ptr(const int32_t type, void* ptr) {
    return remoteness_mask | nan_mask | shifted_type(static_cast<c4_datum_type>(type)) | put_pointer_value(ptr, false);
}

c4_datum_t
c4rt_datum_from_int32(const int32_t i) {
    return nan_mask | shifted_type(C4_Integer) | static_cast<c4_datum_t>(i);
}

c4_datum_t
c4rt_datum_from_int64(const int64_t i) {
    const auto buf = C4_ALLOCATE(int64_t, 1);
    std::construct_at(static_cast<int64_t*>(buf), i);
    return remoteness_mask | nan_mask | shifted_type(C4_Integer) | put_pointer_value(buf, true);
}

c4_datum_t
c4rt_datum_from_int(const int64_t i) {
    if (i >= static_cast<int64_t>(std::numeric_limits<int32_t>::min())
        && i <= static_cast<int64_t>(std::numeric_limits<int32_t>::max()))
        return c4rt_datum_from_int32(static_cast<int32_t>(i));
    return c4rt_datum_from_int64(i);
}

c4_datum_t
c4rt_datum_from_double(const double d) {
    return static_cast<c4_datum_t>(d);
}

namespace {
    c4_datum_t
    datum_from_sso_string(const char* const s, const size_t s_sz) {
        assert(s_sz < 6 && "larger than 5 characters cannot be sso optimized");
        constexpr auto ret = nan_mask | shifted_type(C4_String);
        alignas(c4_datum_t) char buf[8]{};
        std::memcpy(buf, s, s_sz);
        return ret | *reinterpret_cast<c4_datum_t*>(buf);
    }

    c4_datum_t
    datum_from_long_string(const char* const s, const size_t s_sz) {
        constexpr auto ret = remoteness_mask | nan_mask | shifted_type(C4_String);
        const auto full_ptr = C4_ALLOCATE(char, s_sz + 1 + sizeof(std::size_t));
        std::construct_at(static_cast<std::size_t*>(full_ptr), s_sz);
        const auto data_ptr = static_cast<char*>(full_ptr) + sizeof(std::size_t);
        std::memcpy(data_ptr, s, s_sz + 1);
        return ret | put_pointer_value(data_ptr, true);
    }

    c4_datum_t
    datum_from_sized_string(const char* const s, const size_t s_sz) {
        if (s_sz <= 5) return datum_from_sso_string(s, s_sz);
        return datum_from_long_string(s, s_sz);
    }
}

c4_datum_t
c4rt_datum_from_string(const char* const s) {
    const auto s_sz = std::strlen(s);
    return datum_from_sized_string(s, s_sz);
}

c4_datum_t
c4rt_datum_from_string_sz(const char* s, size_t s_sz) {
    return datum_from_sized_string(s, s_sz);
}

namespace {
    void
    datum_free_long_string(char* ptr) {
        C4_FREE(ptr - sizeof(std::size_t));
    }

    bool
    datum_ptr_dynamic(const c4_datum_t d) {
        return static_cast<bool>(d & 1);
    }
}

void
c4rt_datum_free(const c4_datum_t d) {
    if (!c4rt_datum_remoteness_of(d)) return;
    if (!datum_ptr_dynamic(d)) return;
    if (c4rt_datum_type_of(d) == C4_String)
        return datum_free_long_string(static_cast<char*>(get_pointer_value(d)));

    const auto ptr = get_pointer_value(d);
    C4_FREE(ptr);
}

namespace {
    char*
    datum_get_cstr_unck(c4_datum_t& d) {
        return reinterpret_cast<char*>(&d);
    }

    char*
    datum_get_cstr_unck_remote(const c4_datum_t d) {
        return static_cast<char*>(get_pointer_value(d));
    }

    std::size_t
    datum_get_cstr_size_unck(c4_datum_t& d) {
        return strlen(datum_get_cstr_unck(d));
    }

    std::size_t
    datum_get_cstr_size_unck_remote(const c4_datum_t d) {
        return *static_cast<std::size_t*>(
            static_cast<void*>(
                static_cast<char*>(get_pointer_value(d)) - sizeof(std::size_t)
            )
        );
    }

    int32_t
    datum_get_int32_unck(const c4_datum_t datum) {
        return static_cast<int32_t>(payload_mask & datum);
    }

    int32_t
    datum_get_int32_unck_remote(const c4_datum_t datum) {
        const auto ptr = static_cast<int64_t*>(get_pointer_value(datum));
        return static_cast<int32_t>(*ptr);
    }

    int64_t
    datum_get_int64_unck_remote(const c4_datum_t datum) {
        const auto ptr = static_cast<int64_t*>(get_pointer_value(datum));
        return *ptr;
    }
}

int32_t
c4rt_datum_get_int32(const c4_datum_t datum) {
    const auto type = c4rt_datum_type_of(datum);
    const auto remoteness = c4rt_datum_remoteness_of(datum);
    assert(type == C4_Integer && "retrieving int32 from non-integer type is invalid, did you mean to coerce it?");

    if (!remoteness) return datum_get_int32_unck(datum);
    return datum_get_int32_unck_remote(datum);
}

int64_t
c4rt_datum_get_intó4(const c4_datum_t datum) {
    const auto type = c4rt_datum_type_of(datum);
    const auto remoteness = c4rt_datum_remoteness_of(datum);
    assert(type == C4_Integer && "retrieving int64 from non-integer type is invalid, did you mean to coerce it?");

    if (!remoteness) return datum_get_int32_unck(datum);
    return datum_get_int64_unck_remote(datum);
}

double
c4rt_datum_get_double(const c4_datum_t datum) {
    const auto type = c4rt_datum_type_of(datum);
    assert(type == C4_Float && "retrieving double from non-double type is invalid, did you mean to coerce it?");

    return *reinterpret_cast<double*>(datum);
}

const char*
c4rt_datum_get_string(c4_datum_t datum) {
    const auto type = c4rt_datum_type_of(datum);
    const auto remoteness = c4rt_datum_remoteness_of(datum);
    assert(type == C4_String && "retrieving string from non-string type is invalid, did you mean to coerce it?");

    if (!remoteness) return datum_get_cstr_unck(datum);
    return datum_get_cstr_unck_remote(datum);
}

namespace {
    template<class T>
    T
    to_number(char* buf, const size_t buf_sz) {
        T ret{};
        std::from_chars(buf, buf + buf_sz, ret);
        return ret;
    }
}

int32_t
c4rt_datum_coerce_int32(c4_datum_t datum) {
    const auto remoteness = c4rt_datum_remoteness_of(datum);

    switch (const auto type = c4rt_datum_type_of(datum)) {
    case C4_Float: return static_cast<int32_t>(*reinterpret_cast<double*>(datum));
    case C4_Block: return datum != gC4_Empty_Block;
    case C4_Integer: if (!remoteness) return datum_get_int32_unck(datum);
        return datum_get_int32_unck_remote(datum);
    case C4_String: if (!remoteness)
            return to_number<int32_t>(
                datum_get_cstr_unck(datum),
                datum_get_cstr_size_unck(datum)
            );
        return to_number<int32_t>(
            datum_get_cstr_unck_remote(datum),
            datum_get_cstr_size_unck_remote(datum)
        );
    default: UNREACHABLE("invalid datum type {}", static_cast<int>(type));
    }
}

int64_t
c4rt_datum_coerce_int64(c4_datum_t datum) {
    const auto remoteness = c4rt_datum_remoteness_of(datum);

    switch (const auto type = c4rt_datum_type_of(datum)) {
    case C4_Float: return static_cast<int64_t>(*reinterpret_cast<double*>(datum));
    case C4_Block: return datum != gC4_Empty_Block;
    case C4_Integer: if (!remoteness) return datum_get_int32_unck(datum);
        return datum_get_int64_unck_remote(datum);
    case C4_String: if (!remoteness)
            return to_number<int64_t>(
                datum_get_cstr_unck(datum),
                datum_get_cstr_size_unck(datum)
            );
        return to_number<int64_t>(
            datum_get_cstr_unck_remote(datum),
            datum_get_cstr_size_unck_remote(datum)
        );
    default: UNREACHABLE("invalid datum type {}", static_cast<int>(type));
    }
}

double
c4rt_datum_coerce_double(c4_datum_t datum) {
    const auto remoteness = c4rt_datum_remoteness_of(datum);

    switch (const auto type = c4rt_datum_type_of(datum)) {
    case C4_Float: return *reinterpret_cast<double*>(datum);
    case C4_Block: return datum != gC4_Empty_Block;
    case C4_Integer: if (!remoteness) return datum_get_int32_unck(datum);
        return static_cast<double>(datum_get_int64_unck_remote(datum));
    case C4_String: if (!remoteness)
            return to_number<double>(
                datum_get_cstr_unck(datum),
                datum_get_cstr_size_unck(datum)
            );
        return to_number<double>(
            datum_get_cstr_unck_remote(datum),
            datum_get_cstr_size_unck_remote(datum)
        );
    default: UNREACHABLE("invalid datum type {}", static_cast<int>(type));
    }
}

char*
c4rt_datum_coerce_string(c4_datum_t datum) {
    const auto remoteness = c4rt_datum_remoteness_of(datum);

    switch (const auto type = c4rt_datum_type_of(datum)) {
    case C4_Float: return C4_STRDUP(std::format("{}", *reinterpret_cast<double*>(datum)).c_str());
    case C4_Block: if (datum != gC4_Empty_Block) return C4_STRDUP(
            std::format("(block {})", get_pointer_value(datum)).c_str());
        return C4_STRDUP("{}");
    case C4_Integer: if (!remoteness) return C4_STRDUP(std::format("{}", datum_get_int32_unck(datum)).c_str());
        return C4_STRDUP(std::format("{}", datum_get_int64_unck_remote(datum)).c_str());
    case C4_String: if (!remoteness) return C4_STRDUP(datum_get_cstr_unck(datum));
        return C4_STRDUP(datum_get_cstr_unck_remote(datum));
    default: UNREACHABLE("invalid datum type {}", static_cast<int>(type));
    }
}

c4_datum_t
c4rt_datum_dup(const c4_datum_t datum) {
    // since non-remote data is always fully encapsulated inside the 64-bit
    // block, the easiest way to copy it is just copy the int64 type
    if (!c4rt_datum_remoteness_of(datum)) return datum;

    switch (const auto type = c4rt_datum_type_of(datum)) {
    case C4_Float:
        // this can only happen in edge cases where a -NaN occurs. Nevertheless,
        // the solution is trivial:
        return datum;
    case C4_Block: return datum; // todo: figure out the semantics of this
    case C4_Integer: return c4rt_datum_from_int64(c4rt_datum_get_intó4(datum));
    case C4_String:
        // todo: ref-counted CoW strings
        return datum_from_sized_string(
            datum_get_cstr_unck_remote(datum),
            datum_get_cstr_size_unck_remote(datum));
    default: UNREACHABLE("invalid datum type {}", static_cast<int>(type));
    }
}
