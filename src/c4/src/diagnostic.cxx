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
 * src/c4/src/diagnostic --
 *   
 */

#include <algorithm>

#include <c4/diagnostic.hxx>
#include <c4/p2/lex/token_source.hxx>

#include <fmt/format.h>
#include <fmt/color.h>

#include <libassert/assert.hpp>

#include <uni_algo/ranges.h>
#include <uni_algo/ranges_conv.h>

using namespace std::literals;

namespace {
    constexpr auto
    color_from_type(c4::source_diagnostic::diag_type type) {
        switch (type) {
        case c4::source_diagnostic::diag_type::Note: return fmt::terminal_color::magenta;
        case c4::source_diagnostic::diag_type::Warning: return fmt::terminal_color::yellow;
        case c4::source_diagnostic::diag_type::Error: return fmt::terminal_color::bright_red;
        case c4::source_diagnostic::diag_type::Suggestion: return fmt::terminal_color::green;
        }
        UNREACHABLE("invalid diagnostic type", static_cast<int>(type));
    }

    constexpr auto
    prefix_from_type(c4::source_diagnostic::diag_type type) {
        constexpr static std::string_view prefixes[] = {
            "note:"sv,
            "warning:"sv,
            "error:"sv,
            "suggestion:"sv,
        };
        switch (type) {
        case c4::source_diagnostic::diag_type::Note: return styled(prefixes[0], fg(color_from_type(type)));
        case c4::source_diagnostic::diag_type::Warning: return styled(prefixes[1], fg(color_from_type(type)));
        case c4::source_diagnostic::diag_type::Error: return styled(prefixes[2], fg(color_from_type(type)));
        case c4::source_diagnostic::diag_type::Suggestion: return styled(prefixes[3], fg(color_from_type(type)));
        }
        UNREACHABLE("invalid diagnostic type", static_cast<int>(type));
    }

    std::size_t
    utf8_strlen(std::string_view str) {
        auto str_view = str | una::views::utf8;
        return std::distance(str_view.begin(), str_view.end());
    }
}

c4::position::position(p2::token_source* source)
    : _source{source} {
    ASSERT(source, "token source must be a valid source");
}

std::string_view
c4::position::filename() const {
    return _source->file_string();
}

std::string_view
c4::position::range() const {
    const auto range_begin = col_number - 1;
    const auto last_ln_idx = expanded_range.rfind('\n');
    if (last_ln_idx == std::string_view::npos) {
        return expanded_range.substr(range_begin, col_number_end - col_number + 1);
    }
    const auto last_line_start = last_ln_idx + 1;
    const auto last_line_range_end = last_line_start + col_number_end;
    return expanded_range.substr(range_begin, last_line_range_end - range_begin);
}

c4::source_diagnostic
c4::source_diagnostic::suggestion(const std::string& diagnostic,
                                  const position& position,
                                  const std::size_t highlight_length) {
    return {
        diag_type::Suggestion,
        diagnostic,
        position,
        highlight_length
    };
}

c4::source_diagnostic
c4::source_diagnostic::suggestion_for_value(const std::string& diagnostic,
                                            const position& position,
                                            std::string_view value) {
    if (const auto newline_at = value.find('\n');
        newline_at != std::string_view::npos) {
        value = value.substr(0, newline_at + 1);
    }
    return suggestion(diagnostic, position, utf8_strlen(value));
}

c4::source_diagnostic
c4::source_diagnostic::note(const std::string& diagnostic,
                            const position& position,
                            const std::size_t highlight_length) {
    return {
        diag_type::Note,
        diagnostic,
        position,
        highlight_length
    };
}

c4::source_diagnostic
c4::source_diagnostic::note_for_value(const std::string& diagnostic,
                                      const position& position,
                                      std::string_view value) {
    if (const auto newline_at = value.find('\n');
        newline_at != std::string_view::npos) {
        value = value.substr(0, newline_at + 1);
    }
    return note(diagnostic,  position, utf8_strlen(value));
}

c4::source_diagnostic
c4::source_diagnostic::warning(const std::string& diagnostic,
                               const position& position,
                               const std::size_t highlight_length) {
    return {
        diag_type::Warning,
        diagnostic,
        position,
        highlight_length
    };
}

c4::source_diagnostic
c4::source_diagnostic::warning_for_value(const std::string& diagnostic,
                                         const position& position,
                                         std::string_view value) {
    if (const auto newline_at = value.find('\n');
        newline_at != std::string_view::npos) {
        value = value.substr(0, newline_at + 1);
    }
    return warning(diagnostic,  position, utf8_strlen(value));
}

c4::source_diagnostic
c4::source_diagnostic::error(const std::string& diagnostic,
                             const position& position,
                             const std::size_t highlight_length) {
    return {
        diag_type::Error,
        diagnostic,
        position,
        highlight_length
    };
}

c4::source_diagnostic
c4::source_diagnostic::error_for_value(const std::string& diagnostic,
                                       const position& position,
                                       std::string_view value) {
    if (const auto newline_at = value.find('\n');
        newline_at != std::string_view::npos) {
        value = value.substr(0, newline_at + 1);
    }
    return error(diagnostic,  position, utf8_strlen(value));
}

std::string
c4::format_as(const source_diagnostic& diag) {
    char line_number_buf[19]; // floor(log10(9223372036854775807)) + 1
    constexpr static auto space_buffer = std::string_view("                   ");
    static_assert(space_buffer.size() == std::size(line_number_buf));

    const auto [line, row_number, col_number] = diag._position.explode();

    const auto line_number_end = fmt::format_to(line_number_buf, "{}", row_number);
    const std::string_view line_number(line_number_buf, line_number_end - line_number_buf);
    const auto line_offset = space_buffer.substr(0, line_number.size());

    const auto line_before = line
                             | una::views::utf8
                             | una::views::take(col_number - 1)
                             | una::ranges::to_utf8<std::string>();
    const auto line_content = line
                              | una::views::utf8
                              | una::views::drop(col_number - 1)
                              | una::views::take(diag._highlight_length)
                              | una::ranges::to_utf8<std::string>();
    const auto line_after = line
                            | una::views::utf8
                            | una::views::drop(col_number - 1 + diag._highlight_length)
                            | una::ranges::to_utf8<std::string>();

    const auto foreground = color_from_type(diag._diagnostic_type);
    const auto highlight_tail = std::string(std::max(std::size_t{1}, diag._highlight_length) - 1, '~');

    return fmt::format(
        "{}:{}:{}: {} {}\n  {} | {}{}{}\n  {} | {}{}{}",
        diag._filename,
        row_number,
        col_number,
        prefix_from_type(diag._diagnostic_type),
        diag._diagnostic,
        line_number,
        line_before,
        styled(line_content, fg(foreground)),
        line_after,
        line_offset,
        std::string(col_number - 1, ' '),
        styled('^', fg(foreground)),
        styled(highlight_tail, fg(foreground))
    );
}
