/* demo project
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
 * src/c4/include/c4/p2/lex/tokens --
 *   
 */
#ifndef C4_TOKENS_HXX
#define C4_TOKENS_HXX

#include <filesystem>
#include <cstdlib>
#include <type_traits>
#include <string_view>
#include <variant>

#include <fmt/format.h>
#include <fmt/color.h>

#include <c4/diagnostic.hxx>

namespace c4::p2 {
    struct token_source;
}

namespace c4::p2::tokens {
    template<auto>
    using void_val_t = void;

    template<class, class = void>
    struct can_match_newline : std::false_type { };

    template<class Token>
    struct can_match_newline<Token,
                             void_val_t<Token::can_match_newline>>
            : std::bool_constant<Token::can_match_newline> { };

    template<class, class = void>
    struct capture_groups
            : std::integral_constant<std::uint32_t, 0 + 1> { };

    template<class Token>
    struct capture_groups<Token,
                          void_val_t<Token::group_count>>
            : std::integral_constant<std::uint32_t, Token::group_count + 1> { };

    struct token_base {
        [[nodiscard]] const char*
        begin() const noexcept { return _begin; }

        [[nodiscard]] const char*
        end() const noexcept { return _end; }

        [[nodiscard]] std::string_view
        value() const noexcept {
            return {_begin, static_cast<std::string_view::size_type>(_end - _begin)};
        }

        [[nodiscard]] position
        token_position() const noexcept { return _position; }

        [[nodiscard]] std::string
        source_name() const noexcept;

    protected:
        token_base(token_source* source,
                   const position& position,
                   std::string_view range);

    private:
        friend token_source;
        token_source* _source;

    protected:
        position _position;
        const char* _begin;
        const char* _end;
    };

#define C4P2_DEFAULT_TOKEN(name) \
    private: \
        friend token_source; \
        name(token_source* source, \
             const position& position, \
             const std::string_view range) \
          : token_base{source, position, range} { }

#define C4P2_DEFAULT_TOKEN_DECL(name) \
    private: \
        friend token_source; \
        name(token_source* source, \
             const position& position, \
             const std::string_view range);

    struct whitespace : token_base {
        constexpr static std::string_view regex = R"(\A\s+)";
        constexpr static bool can_match_newline = true;

        C4P2_DEFAULT_TOKEN(whitespace)
    };

    struct comment : token_base {
        constexpr static std::string_view regex = R"(\A#.+\n)";
        constexpr static bool can_match_newline = true;

        C4P2_DEFAULT_TOKEN(comment)
    };

    struct bare_symbol : token_base {
        constexpr static std::string_view regex = R"(\A\w+)";

        [[nodiscard]] std::string_view
        name() const noexcept { return value(); }

        [[nodiscard]] static unsigned
        arity() noexcept { return 0; }

        C4P2_DEFAULT_TOKEN(bare_symbol)
    };

    struct symbol : token_base {
        constexpr static std::string_view regex = R"(\A(\p{L}\p{Xwd}*)/(\d+))";
        constexpr static std::uint32_t group_count = 2;

        [[nodiscard]] std::string_view
        name() const noexcept {
            return std::string_view{_name_begin, static_cast<std::string_view::size_type>(_name_end - _name_begin)};
        }

        [[nodiscard]] unsigned
        arity() const noexcept { return _arity; }

    private:
        friend token_source;

        symbol(token_source* source,
               const position& position,
               std::string_view range,
               std::string_view name_range,
               std::string_view arity_range);

        const char* _name_begin;
        const char* _name_end;
        unsigned _arity;
    };

    struct let : token_base {
        constexpr static std::string_view regex = R"(\Alet\b)";
        C4P2_DEFAULT_TOKEN_DECL(let)
    };

    struct lbrace : token_base {
        constexpr static std::string_view regex = R"(\A\{)";
        C4P2_DEFAULT_TOKEN_DECL(lbrace)
    };

    struct rbrace : token_base {
        constexpr static std::string_view regex = R"(\A\})";
        C4P2_DEFAULT_TOKEN_DECL(rbrace)
    };

    struct lparen : token_base {
        constexpr static std::string_view regex = R"(\A\()";
        C4P2_DEFAULT_TOKEN_DECL(lparen)
    };

    struct rparen : token_base {
        constexpr static std::string_view regex = R"(\A\))";
        C4P2_DEFAULT_TOKEN_DECL(rparen)
    };

    struct arrow : token_base {
        constexpr static std::string_view regex = R"(\A->)";
        C4P2_DEFAULT_TOKEN_DECL(arrow)
    };

    struct ampersand : token_base {
        constexpr static std::string_view regex = R"(\A&\()";
        C4P2_DEFAULT_TOKEN_DECL(ampersand)
    };

    struct semicolon : token_base {
        constexpr static std::string_view regex = R"(\A;)";
        C4P2_DEFAULT_TOKEN_DECL(semicolon)
    };

    struct backslash : token_base {
        constexpr static std::string_view regex = R"(\A\\)";
        C4P2_DEFAULT_TOKEN_DECL(backslash)
    };

    struct arity_marker : token_base {
        constexpr static std::string_view regex = R"(\A\)/(\d+))";
        constexpr static std::uint32_t group_count = 1;

        [[nodiscard]] unsigned
        arity() const noexcept { return _arity; }

    private:
        friend token_source;

        arity_marker(token_source* source,
                     const position& pos,
                     std::string_view range,
                     std::string_view arity_range);

        unsigned _arity;
    };

    struct operator_ : token_base {
        // note: keep in sync with fn_operator::regex
        constexpr static std::string_view regex = R"(\A[-+*%&|~!?,.:^@#`<>=]+)";

        C4P2_DEFAULT_TOKEN(operator_)
    };

    struct fn_operator : token_base {
        // note: keep in sync with operator_::regex
        constexpr static std::string_view regex = R"(\A\([-+*%&|~!?,.:^@#`<>=]+\))";

        C4P2_DEFAULT_TOKEN(fn_operator)
    };

    struct string_literal : token_base {
        constexpr static std::string_view regex = R"(\A"[^"]*?")";
        constexpr static bool can_match_newline = true;

        [[nodiscard]] std::string_view
        string_value() const noexcept { return value().substr(1, value().size() - 2); }

        C4P2_DEFAULT_TOKEN(string_literal)
    };

    struct integer_literal : token_base {
        constexpr static std::string_view regex = R"(\A[-+]?[0-9]+)";

        [[nodiscard]] std::int64_t
        int_value() const { return _int_value; }

    private:
        friend token_source;

        integer_literal(token_source* source,
                        const position& position,
                        std::string_view range);

        std::int64_t _int_value;
    };

    struct float_literal : token_base {
        constexpr static std::string_view regex = R"(\A[-+]?[0-9]+\.[0-9]+)";

        [[nodiscard]] double
        float_value() const { return _float_value; }

    private:
        friend token_source;

        float_literal(token_source* source,
                      const position& position,
                      std::string_view range);

        double _float_value;
    };

    using token_type = std::variant<
        whitespace, //
        comment, // #..\n

        ampersand, // &(
        arity_marker, // )/1
        fn_operator, // (+)
        lparen, // (
        rparen, // )
        lbrace, // {
        rbrace, // }
        arrow, // ->
        backslash, // \ <- space needed to not escape linebreak
        let, // let

        integer_literal, // 42
        float_literal, // 12.1
        operator_, // >=>
        string_literal, // "asd"
        symbol, // sym/1
        bare_symbol // sym
    >;
}

#endif
