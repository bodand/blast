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

#include <c4/diagnostic.hxx>

namespace c4::p2 {
    struct token_source;
}

namespace c4::p2::tokens {
    struct token_base {
        [[nodiscard]] const char*
        begin() const noexcept { return _begin; }

        [[nodiscard]] const char*
        end() const noexcept { return _end; }

        [[nodiscard]] std::string_view
        value() const noexcept {
            return {_begin, static_cast<std::string_view::size_type>(_end - _begin)};
        }

        [[nodiscard]] auto
        size() const noexcept {
            return static_cast<std::string_view::size_type>(_end - _begin);
        }

        [[nodiscard]] const position&
        token_position() const noexcept { return _position; }

    protected:
        token_base(const position& position,
                   std::string_view range);

        position _position;
        const char* _begin;
        const char* _end;

    private:
        friend token_source;
        token_source* _source; // todo remove
    };

#define C4P2_DEFAULT_TOKEN(name) \
    private: \
        friend token_source; \
        name(const position& position, \
             const std::string_view range) \
          : token_base{position, range} { }

#define C4P2_DEFAULT_TOKEN_DECL(name) \
    private: \
        friend token_source; \
        name(const position& position, \
             const std::string_view range);

    template<class T>
    struct literal_token : token_base {
        template<class X>
        [[nodiscard]] X
        as_value() const noexcept = delete;

        template<>
        [[nodiscard]] T
        as_value<T>() const noexcept { return _value; }

    protected:
        T _value;

        literal_token(T&& value,
                      const position& position,
                      const std::string_view range)
            : token_base{position, range}
            , _value(std::move(value)) { }
    };

    struct whitespace final : token_base {
        constexpr static std::string_view token_name = "whitespace";
        constexpr static std::string_view regex = R"(\A\s+)";
        constexpr static auto can_match_newline = true;

        C4P2_DEFAULT_TOKEN(whitespace)
    };

    struct comment final : token_base {
        constexpr static std::string_view token_name = "comment";
        constexpr static std::string_view regex = R"(\A#[^\n]*\n)";
        constexpr static bool can_match_newline = true;

        C4P2_DEFAULT_TOKEN(comment)
    };

    struct bare_symbol final : token_base {
        constexpr static std::string_view token_name = "bare symbol";
        constexpr static std::string_view regex = R"(\A\p{L}\p{Xwd}*)";

        [[nodiscard]] std::string_view
        name() const noexcept { return value(); }

        [[nodiscard]] static unsigned
        arity() noexcept { return 0; }

        C4P2_DEFAULT_TOKEN(bare_symbol)
    };

    struct symbol final : token_base {
        constexpr static std::string_view token_name = "symbol";
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

        symbol(const position& position,
               std::string_view range,
               std::string_view name_range,
               std::string_view arity_range);

        const char* _name_begin;
        const char* _name_end;
        unsigned _arity;
    };

    struct let final : token_base {
        constexpr static std::string_view token_name = "let";
        constexpr static std::string_view regex = R"(\Alet\b)";
        C4P2_DEFAULT_TOKEN_DECL(let)
    };

    struct lbrace final : token_base {
        constexpr static std::string_view token_name = "opening brace '{'";
        constexpr static std::string_view string = "{";
        C4P2_DEFAULT_TOKEN_DECL(lbrace)
    };

    struct rbrace final : token_base {
        constexpr static std::string_view token_name = "closing brace '}'";
        constexpr static std::string_view string = "}";
        C4P2_DEFAULT_TOKEN_DECL(rbrace)
    };

    struct lparen final : token_base {
        constexpr static std::string_view token_name = "opening parenthesis '('";
        constexpr static std::string_view string = "(";
        C4P2_DEFAULT_TOKEN_DECL(lparen)
    };

    struct rparen final : token_base {
        constexpr static std::string_view token_name = "closing parenthesis ')'";
        constexpr static std::string_view string = ")";
        C4P2_DEFAULT_TOKEN_DECL(rparen)
    };

    struct pipe final : token_base {
        constexpr static std::string_view token_name = "parameter marker '|'";
        constexpr static std::string_view string = "|";
        C4P2_DEFAULT_TOKEN_DECL(pipe)
    };

    struct ampersand final : token_base {
        constexpr static std::string_view token_name = "indirect call opening ampersand '&('";
        constexpr static std::string_view string = "&(";
        C4P2_DEFAULT_TOKEN_DECL(ampersand)
    };

    struct semicolon final : token_base {
        constexpr static std::string_view token_name = "semicolon ';'";
        constexpr static std::string_view string = ";";
        C4P2_DEFAULT_TOKEN_DECL(semicolon)
    };

    struct backslash final : token_base {
        constexpr static std::string_view token_name = "backslash '\\'";
        constexpr static std::string_view string = R"(\)";
        C4P2_DEFAULT_TOKEN_DECL(backslash)
    };

    struct arity_marker final : token_base {
        constexpr static std::string_view token_name = "indirect call closing arity marker";
        constexpr static std::string_view regex = R"(\A\)/(\d+))";
        constexpr static std::uint32_t group_count = 1;

        [[nodiscard]] unsigned
        arity() const noexcept { return _arity; }

    private:
        friend token_source;

        arity_marker(const position& pos,
                     std::string_view range,
                     std::string_view arity_range);

        unsigned _arity;
    };

    struct operator_ final : token_base {
        constexpr static std::string_view token_name = "operator";
        // note: keep in sync with fn_operator::regex
        constexpr static std::string_view regex = R"(\A[-+*/%&|~!?,.:^@#`<>=]+)";

        C4P2_DEFAULT_TOKEN(operator_)
    };

    struct operator_symbol final : token_base {
        constexpr static std::string_view token_name = "operator symbol";
        // note: keep in sync with fn_operator::regex
        constexpr static std::string_view regex = R"(\A\(([-+*/%&|~!?,.:^@#`<>=]+)\)/(\d+))";
        constexpr static std::uint32_t group_count = 2;

        [[nodiscard]] std::string_view
        name() const noexcept {
            return std::string_view{_name_begin, static_cast<std::string_view::size_type>(_name_end - _name_begin)};
        }

        [[nodiscard]] unsigned
        arity() const noexcept { return _arity; }

    private:
        friend token_source;

        operator_symbol(const position& position,
                        std::string_view range,
                        std::string_view name_range,
                        std::string_view arity_range);

        const char* _name_begin;
        const char* _name_end;
        unsigned _arity;
    };

    struct fn_operator final : token_base {
        constexpr static std::string_view token_name = "fn-syntax operator";
        // note: keep in sync with operator_::regex
        constexpr static std::string_view regex = R"(\A\([-+*/%&|~!?,.:^@#`<>=]+\))";

        [[nodiscard]] std::string_view
        name() const noexcept {
            return std::string_view{
                _begin + 1,
                static_cast<std::string_view::size_type>((_end - 1) - (_begin + 1))
            };
        }

        [[nodiscard]] unsigned
        arity() const noexcept { return 1; }

        C4P2_DEFAULT_TOKEN(fn_operator)
    };

    struct string_literal final : literal_token<std::string_view> {
        constexpr static std::string_view token_name = "string literal";
        constexpr static std::string_view regex = R"(\A"[^"]*?")";
        constexpr static auto can_match_newline = true;

    private:
        friend token_source;

        string_literal(const position& position,
                       std::string_view range)
            : literal_token<std::string_view>(range.substr(1, range.size() - 2),
                                              position,
                                              range) { }
    };

    struct integer_literal final : literal_token<std::int64_t> {
        constexpr static std::string_view token_name = "integer literal";
        constexpr static std::string_view regex = R"(\A[-+]?[0-9]+)";

    private:
        friend token_source;

        integer_literal(const position& position,
                        std::string_view range);
    };

    struct float_literal final : literal_token<double> {
        constexpr static std::string_view token_name = "float literal";
        constexpr static std::string_view regex = R"(\A[-+]?[0-9]+\.[0-9]+)";

    private:
        friend token_source;

        float_literal(const position& position,
                      std::string_view range);
    };

    struct eof final : token_base {
        constexpr static std::string_view token_name = "end of file";
        constexpr static std::string_view string = R"(\A\Z)";
        C4P2_DEFAULT_TOKEN(eof)
    };

    struct unknown final : token_base {
        constexpr static std::string_view token_name = "unknown";
        constexpr static std::string_view regex = R"(\A\S+)";
        C4P2_DEFAULT_TOKEN(unknown)
    };

    using token_type = std::variant<
        whitespace, //
        comment, // #..\n

        operator_symbol, // (+)/2
        fn_operator, // (+)

        ampersand, // &(
        arity_marker, // )/1
        lparen, // (
        rparen, // )
        lbrace, // {
        rbrace, // }
        pipe, // |
        backslash, // \ <- space needed to not escape linebreak
        let, // let

        float_literal, // 12.1
        integer_literal, // 42
        operator_, // >=>
        string_literal, // "asd"
        symbol, // sym/1
        bare_symbol, // sym
        eof,
        unknown
    >;
}

#endif
