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
 * test/c4rt/datum --
 *   
 */

#include <c4rt/datum.h>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("datum reports correct type") {
    SECTION("int32") {
        const auto datum = c4rt_datum_from_int32(42);
        const auto type = c4rt_datum_type_of(datum);
        CHECK(type == C4_Integer);
        c4rt_datum_free(datum);
    }

    SECTION("int64") {
        const auto datum = c4rt_datum_from_int64(42);
        const auto type = c4rt_datum_type_of(datum);
        CHECK(type == C4_Integer);
        c4rt_datum_free(datum);
    }

    SECTION("double") {
        const auto datum = c4rt_datum_from_double(42.0);
        const auto type = c4rt_datum_type_of(datum);
        CHECK(type == C4_Float);
        c4rt_datum_free(datum);
    }

    SECTION("double (NaN)") {
        const auto datum = c4rt_datum_from_double(NAN);
        const auto type = c4rt_datum_type_of(datum);
        CHECK(type == C4_Float);
        c4rt_datum_free(datum);
    }

    SECTION("long string") {
        const auto datum = c4rt_datum_from_string("long string value");
        const auto type = c4rt_datum_type_of(datum);
        CHECK(type == C4_String);
        c4rt_datum_free(datum);
    }

    SECTION("short string") {
        const auto datum = c4rt_datum_from_string("smol");
        const auto type = c4rt_datum_type_of(datum);
        CHECK(type == C4_String);
        c4rt_datum_free(datum);
    }
}

TEST_CASE("datum reports correct remoteness") {
    SECTION("int32") {
        const auto datum = c4rt_datum_from_int32(42);
        CHECK_FALSE(c4rt_datum_remoteness_of(datum));
        c4rt_datum_free(datum);
    }

    SECTION("int64") {
        const auto datum = c4rt_datum_from_int64(42);
        CHECK(c4rt_datum_remoteness_of(datum));
        c4rt_datum_free(datum);
    }

    SECTION("double") {
        const auto datum = c4rt_datum_from_double(42.0);
        CHECK_FALSE(c4rt_datum_remoteness_of(datum));
        c4rt_datum_free(datum);
    }

    SECTION("long string") {
        const auto datum = c4rt_datum_from_string("long string value");
        CHECK(c4rt_datum_remoteness_of(datum));
        c4rt_datum_free(datum);
    }

    SECTION("short string") {
        const auto datum = c4rt_datum_from_string("smol");
        CHECK_FALSE(c4rt_datum_remoteness_of(datum));
        c4rt_datum_free(datum);
    }
}

TEST_CASE("get in32 returns int given to create int32") {
    const auto datum = c4rt_datum_from_int32(42);
    const auto int32 = c4rt_datum_get_int32(datum);
    CHECK(int32 == 42);
}

TEST_CASE("coerce int32 returns int32 value") {
    SECTION("int32") {
        const auto datum = c4rt_datum_from_int32(42);
        const auto int32 = c4rt_datum_coerce_int32(datum);
        CHECK(int32 == 42);
    }

    SECTION("int64") {
        const auto datum = c4rt_datum_from_int64(42);
        const auto int32 = c4rt_datum_coerce_int32(datum);
        CHECK(int32 == 42);
        c4rt_datum_free(datum);
    }

    SECTION("double") {
        const auto datum = c4rt_datum_from_double(42.0);
        const auto int32 = c4rt_datum_coerce_int32(datum);
        CHECK(int32 == 42);
        c4rt_datum_free(datum);
    }

    SECTION("long string (numeric)") {
        const auto datum = c4rt_datum_from_string("42long string value");
        const auto int32 = c4rt_datum_coerce_int32(datum);
        CHECK(int32 == 42);
        c4rt_datum_free(datum);
    }

    SECTION("long string (not numeric)") {
        const auto datum = c4rt_datum_from_string("long string value");
        const auto int32 = c4rt_datum_coerce_int32(datum);
        CHECK(int32 == 0);
        c4rt_datum_free(datum);
    }

    SECTION("short string (numeric)") {
        const auto datum = c4rt_datum_from_string("42");
        const auto int32 = c4rt_datum_coerce_int32(datum);
        CHECK(int32 == 42);
        c4rt_datum_free(datum);
    }

    SECTION("short string (not numeric)") {
        const auto datum = c4rt_datum_from_string("smol");
        const auto int32 = c4rt_datum_coerce_int32(datum);
        CHECK(int32 == 0);
        c4rt_datum_free(datum);
    }
}
