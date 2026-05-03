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
 * test/c4/p2/lex/lexer --
 *   
 */

#include <c4/p2/lex/lexer.hxx>

#include <catch2/catch_test_macros.hpp>

namespace {
	template<class T>
	struct token_finder {
		bool
		operator()(const T&) const noexcept { return true; }

		bool
		operator()(const auto&) const noexcept { return false; }
	};

	struct position_extractor {
		template<class T>
		const auto&
		operator()(const T& tok) const noexcept {
			return tok.token_position();
		}
	};
};

#define KNOWN_TOKEN_TEST(name, str, tok) \
    SECTION(name) { \
        constexpr std::string_view buf(str, sizeof(str) - 1); \
        c4::p2::lexer lexer("", buf.data(), buf.data() + buf.size()); \
        const auto token = lexer.next(); \
        CHECK(std::visit(token_finder<c4::p2::tokens::tok>{}, token)); \
    }

TEST_CASE (
"single known token is lexed"
)
 {
    KNOWN_TOKEN_TEST("opening ampersand", "&(", ampersand)
    KNOWN_TOKEN_TEST("call arity marker", ")/42", arity_marker)
    KNOWN_TOKEN_TEST("fn operator", "(+/-)", fn_operator)
    KNOWN_TOKEN_TEST("left parenthesis", "(", lparen)
    KNOWN_TOKEN_TEST("right parenthesis", ")", rparen)
    KNOWN_TOKEN_TEST("left brace", "{", lbrace)
    KNOWN_TOKEN_TEST("right brace", "}", rbrace)
    KNOWN_TOKEN_TEST("pipe", "|", pipe)
    KNOWN_TOKEN_TEST("backslash", "\\", backslash)
    KNOWN_TOKEN_TEST("let", "let", let)
    KNOWN_TOKEN_TEST("integer literal", "42", integer_literal)
    KNOWN_TOKEN_TEST("floating point literal", "42.0", float_literal)
    KNOWN_TOKEN_TEST("operator", ">=>", operator_)
    KNOWN_TOKEN_TEST("string literal", R"("some string")", string_literal)
    KNOWN_TOKEN_TEST("string literal (multiline)", "\"some\nstring\"", string_literal)
    KNOWN_TOKEN_TEST("symbol", "sym/1", symbol)
    KNOWN_TOKEN_TEST("bare symbol", "sym", bare_symbol)
}

TEST_CASE (
"unknown tokens are packaged into unknown"
)
 {
    KNOWN_TOKEN_TEST("unknown hex", "\x04\x05\x06", unknown)
    KNOWN_TOKEN_TEST("zero bytes", "\0\0\0x", unknown)
}

TEST_CASE (
"simple token lexed with initial starting position"
)
 {
    constexpr std::string_view buf("let x = 42");
    c4::p2::lexer lexer("", buf.data(), buf.data() + buf.size());
    const auto token = lexer.next();

    const auto pos = std::visit(position_extractor{}, token);
    CHECK(pos.col_number == 1);
    CHECK(pos.row_number == 1);
    CHECK(pos.col_number_end == 3);
    CHECK(pos.row_number_end == 1);
}

TEST_CASE (
"simple token lexed with padded starting position"
)
 {
    constexpr std::string_view buf("    let x = 42");
    c4::p2::lexer lexer("", buf.data(), buf.data() + buf.size());
    lexer.next(); // whitespace
    const auto token = lexer.next();

    const auto pos = std::visit(position_extractor{}, token);
    CHECK(pos.col_number == 5);
    CHECK(pos.row_number == 1);
    CHECK(pos.col_number_end == 7);
    CHECK(pos.row_number_end == 1);
}

TEST_CASE (
"simple token lexed with newline padded starting position"
)
 {
    constexpr std::string_view buf("\n\nlet x = 42");
    c4::p2::lexer lexer("", buf.data(), buf.data() + buf.size());
    lexer.next(); // whitespace
    const auto token = lexer.next();

    const auto pos = std::visit(position_extractor{}, token);
    CHECK(pos.col_number == 1);
    CHECK(pos.row_number == 3);
    CHECK(pos.col_number_end == 3);
    CHECK(pos.row_number_end == 3);
}

TEST_CASE (
"multiline token lexed with initial starting position"
)
 {
    constexpr std::string_view buf(R"("
multiline
")");
    c4::p2::lexer lexer("", buf.data(), buf.data() + buf.size());
    const auto token = lexer.next();

    const auto pos = std::visit(position_extractor{}, token);
    CHECK(pos.col_number == 1);
    CHECK(pos.row_number == 1);
    CHECK(pos.col_number_end == 1);
    CHECK(pos.row_number_end == 3);
}

TEST_CASE (
"multiline token lexed with padded starting position"
)
 {
    constexpr std::string_view buf(R"(    "
multiline
")");
    c4::p2::lexer lexer("", buf.data(), buf.data() + buf.size());
    lexer.next(); // whitespace
    const auto token = lexer.next();

    const auto pos = std::visit(position_extractor{}, token);
    CHECK(pos.col_number == 5);
    CHECK(pos.row_number == 1);
    CHECK(pos.col_number_end == 1);
    CHECK(pos.row_number_end == 3);
}

TEST_CASE (
"multiline token lexed with newline padded starting position"
)
 {
    constexpr std::string_view buf(R"(

"
multiline
")");
    c4::p2::lexer lexer("", buf.data(), buf.data() + buf.size());
    lexer.next(); // whitespace
    const auto token = lexer.next();

    const auto pos = std::visit(position_extractor{}, token);
    CHECK(pos.col_number == 1);
    CHECK(pos.row_number == 3);
    CHECK(pos.col_number_end == 1);
    CHECK(pos.row_number_end == 5);
}

TEST_CASE (
"simple token's expanded_range is single line with token"
)
 {
    constexpr std::string_view buf("let x = 42");
    c4::p2::lexer lexer("", buf.data(), buf.data() + buf.size());
    const auto token = lexer.next();

    const auto pos = std::visit(position_extractor{}, token);
    CHECK(pos.expanded_range == buf);
}

TEST_CASE (
"simple token's expanded_range is single line ith token if not on col0"
)
 {
    constexpr std::string_view buf("    let x = 42");
    c4::p2::lexer lexer("", buf.data(), buf.data() + buf.size());
    lexer.next(); // whitespace
    const auto token = lexer.next();

    const auto pos = std::visit(position_extractor{}, token);
    CHECK(pos.expanded_range == buf);
}

TEST_CASE (
"simple token's expanded_range is single line with token if not line0"
)
 {
    constexpr std::string_view buf("\n\nlet x = 42");
    c4::p2::lexer lexer("", buf.data(), buf.data() + buf.size());
    lexer.next(); // whitespace
    const auto token = lexer.next();

    const auto pos = std::visit(position_extractor{}, token);
    CHECK(pos.expanded_range == "let x = 42");
}

TEST_CASE (
"second simple token's expanded_range is single line with token"
)
 {
    constexpr std::string_view buf("let xyz = 42");
    c4::p2::lexer lexer("", buf.data(), buf.data() + buf.size());
    lexer.next(); // whitespace
    lexer.next(); // let
    lexer.next(); // whitespace
    const auto token = lexer.next();

    const auto pos = std::visit(position_extractor{}, token);
    CHECK(pos.expanded_range == buf);
}

TEST_CASE (
"multiline token's expanded_range is all lines with token"
)
 {
    constexpr std::string_view buf(R"("
multiline
")");
    c4::p2::lexer lexer("", buf.data(), buf.data() + buf.size());
    const auto token = lexer.next();

    const auto pos = std::visit(position_extractor{}, token);
    CHECK(pos.expanded_range == buf);
}

TEST_CASE (
"multiline token's expanded_range is all lines with token if not on col0"
)
 {
    constexpr std::string_view buf(R"(    "
multiline
")");
    c4::p2::lexer lexer("", buf.data(), buf.data() + buf.size());
    lexer.next(); // whitespace
    const auto token = lexer.next();

    const auto pos = std::visit(position_extractor{}, token);
    CHECK(pos.expanded_range == buf);
}

TEST_CASE (
"multiline token's expanded_range is all lines with token if not on line0"
)
 {
    constexpr std::string_view buf(R"(

 "
multiline
")");
    c4::p2::lexer lexer("", buf.data(), buf.data() + buf.size());
    lexer.next(); // whitespace
    const auto token = lexer.next();

    const auto pos = std::visit(position_extractor{}, token);
    CHECK(pos.expanded_range == R"( "
multiline
")");
}

TEST_CASE (
"simple token's range is the token"
)
 {
    constexpr std::string_view buf("let x = 42");
    c4::p2::lexer lexer("", buf.data(), buf.data() + buf.size());
    const auto token = lexer.next();

    const auto pos = std::visit(position_extractor{}, token);
    CHECK(pos.range() == "let");
}

TEST_CASE (
"simple token's range is the token if not on col0"
)
 {
    constexpr std::string_view buf("    let x = 42");
    c4::p2::lexer lexer("", buf.data(), buf.data() + buf.size());
    lexer.next(); // whitespace
    const auto token = lexer.next();

    const auto pos = std::visit(position_extractor{}, token);
    CHECK(pos.range() == "let");
}

TEST_CASE (
"simple token's range is the token if not line0"
)
 {
    constexpr std::string_view buf("\n\nlet x = 42");
    c4::p2::lexer lexer("", buf.data(), buf.data() + buf.size());
    lexer.next(); // whitespace
    const auto token = lexer.next();

    const auto pos = std::visit(position_extractor{}, token);
    CHECK(pos.range() == "let");
}

TEST_CASE (
"second simple token's range is the token"
)
 {
    constexpr std::string_view buf("let xyz = 42");
    c4::p2::lexer lexer("", buf.data(), buf.data() + buf.size());
    lexer.next(); // let
    lexer.next(); // whitespace
    const auto token = lexer.next();

    const auto pos = std::visit(position_extractor{}, token);
    CHECK(pos.range() == "xyz");
}

TEST_CASE (
"multiline token's range is the token"
)
 {
    constexpr std::string_view buf(R"("
multiline
")");
    c4::p2::lexer lexer("", buf.data(), buf.data() + buf.size());
    const auto token = lexer.next();

    const auto pos = std::visit(position_extractor{}, token);
    CHECK(pos.range() == buf);
}

TEST_CASE (
"multiline token's range is the token if not on col0"
)
 {
    constexpr std::string_view buf(R"(    "
multiline
")");
    c4::p2::lexer lexer("", buf.data(), buf.data() + buf.size());
    lexer.next(); // whitespace
    const auto token = lexer.next();

    const auto pos = std::visit(position_extractor{}, token);
    CHECK(pos.range() == R"("
multiline
")");
}

TEST_CASE (
"multiline token's range is the token if not on line0"
)
 {
    constexpr std::string_view buf(R"(

 "
multiline
")");
    c4::p2::lexer lexer("", buf.data(), buf.data() + buf.size());
    lexer.next(); // whitespace
    const auto token = lexer.next();

    const auto pos = std::visit(position_extractor{}, token);
    CHECK(pos.range() == R"("
multiline
")");
}
