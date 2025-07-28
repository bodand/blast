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
 * src/c4/include/c4/diagnostic --
 *   Diagnostic object. Can be printed with fmt to produce mostly nice looking
 *   diagnostic messages.
 */
#ifndef C4_DIAGNOSTIC_HXX
#define C4_DIAGNOSTIC_HXX

#include <string_view>
#include <cstdint>
#include <string>
#include <type_traits>
#include <tuple>
#include <utility>

#include <fmt/base.h>

namespace c4::p2 {
    struct token_source;
}

namespace c4 {
    struct position {
        explicit
        position(p2::token_source* source);

        std::string_view line;
        std::size_t row_number{1};
        std::size_t col_number{1};
        std::size_t row_number_end{1};
        std::size_t col_number_end{1};
        std::string_view expanded_range{};

        [[nodiscard]] position
        snapshot() const noexcept(std::is_nothrow_copy_constructible_v<position>) {
            return *this;
        }

        [[nodiscard]] std::string_view
        filename() const;

        [[nodiscard]] p2::token_source*
        source() const { return _source; }

        [[nodiscard]] auto
        explode() const {
            return std::make_tuple(line, row_number, col_number);
        }

        [[nodiscard]] std::string_view
        range() const;

        [[nodiscard]] bool
        is_single_line() const noexcept { return row_number == row_number_end; }

    private:
        p2::token_source* _source{};
    };

    struct source_diagnostic {
        enum class diag_type {
            Note,
            Warning,
            Error,
            Suggestion
        };

        template<class... Args>
        [[nodiscard]] static source_diagnostic
        suggestion(const position& position, fmt::format_string<Args...> diagnostic, Args&&... args) {
            return {
                diag_type::Suggestion,
                fmt::format(diagnostic, std::forward<Args>(args)...),
                position
            };
        }

        template<class... Args>
        [[nodiscard]] static source_diagnostic
        note(const position& position, fmt::format_string<Args...> diagnostic, Args&&... args) {
            return {
                diag_type::Note,
                fmt::format(diagnostic, std::forward<Args>(args)...),
                position
            };
        }

        template<class... Args>
        [[nodiscard]] static source_diagnostic
        warning(const position& position, fmt::format_string<Args...> diagnostic, Args&&... args) {
            return {
                diag_type::Warning,
                fmt::format(diagnostic, std::forward<Args>(args)...),
                position
            };
        }

        template<class... Args>
        [[nodiscard]] static source_diagnostic
        error(const position& position, fmt::format_string<Args...> diagnostic, Args&&... args) {
            return {
                diag_type::Error,
                fmt::format(diagnostic, std::forward<Args>(args)...),
                position
            };
        }

    private:
        source_diagnostic(const diag_type type,
                          std::string diagnostic,
                          const position& position,
                          const std::size_t highlight_length = 0)
            : _diagnostic_type{type}
            , _diagnostic{std::move(diagnostic)}
            , _filename{position.filename()}
            , _position{position}
            , _highlight_length{highlight_length} { }

        friend struct fmt::formatter<c4::source_diagnostic>;

        diag_type _diagnostic_type;
        std::string _diagnostic;
        std::string_view _filename;
        position _position;
        std::size_t _highlight_length{};
    };
}

template<>
struct fmt::formatter<c4::source_diagnostic> {
    constexpr auto
    parse(format_parse_context& ctx) { return std::ranges::find(ctx, '}'); }

    format_context::iterator
    format(const c4::source_diagnostic& diag, fmt::format_context& ctx) const;
};


#endif
