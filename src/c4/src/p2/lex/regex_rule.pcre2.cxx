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
 * src/c4/src/p2/lex/regex_rule --
 *   PCRE2 backend for regex rule matching. Uses JIT compilation and works
 *   basically everywhere therefore it is the default and fallback.
 */

#include <algorithm>

#include <c4/p2/lex/regex_rule.hxx>

#include <c4/p2/lex/token_source.hxx>
#include <c4/p2/lex/tokens.hxx>

#include <pcre2.h>
#include <libassert/assert.hpp>
#include <uni_algo/ranges.h>
#include <uni_algo/ranges_conv.h>

c4::p2::regex_context::regex_context() {
    _impl = pcre2_compile_context_create(nullptr);
    const auto cc = static_cast<pcre2_compile_context*>(_impl);
    pcre2_set_newline(cc, PCRE2_NEWLINE_ANYCRLF);
}

c4::p2::regex_context::~regex_context() noexcept {
    if (!_impl) return;
    const auto cc = static_cast<pcre2_compile_context*>(_impl);
    pcre2_compile_context_free(cc);
}

c4::p2::regex_match_context::regex_match_context(const std::uint32_t capture_groups) {
    _impl = pcre2_match_data_create(capture_groups, nullptr);
}

c4::p2::regex_match_context::~regex_match_context() noexcept {
    if (!_impl) return;
    const auto match_data = static_cast<pcre2_match_data*>(_impl);
    pcre2_match_data_free(match_data);
}

c4::p2::regex_rule::regex_rule(token_source& token_source,
                               regex_context& ctx,
                               const std::string_view regex,
                               std::uint32_t groups_expected)
    : _token_source{&token_source}
    , _ctx{&ctx} {
    DEBUG_ASSERT(groups_expected >= 1,
                 "at least one match group is always to be expected: the matched range");

    const auto cc = static_cast<pcre2_compile_context*>(_ctx->_impl);

    int ec{};
    PCRE2_SIZE error_at{};
    _impl_handle = pcre2_compile(reinterpret_cast<PCRE2_SPTR>(regex.data()), regex.size(),
                                 PCRE2_UTF,
                                 &ec, &error_at,
                                 cc);
    const auto re = static_cast<pcre2_code*>(_impl_handle);

    if (!re) {
        PCRE2_UCHAR error_buf[128];
        const auto returned = pcre2_get_error_message(ec, error_buf, std::size(error_buf));
        const auto err_msg = returned > 0
                             ? std::string_view(reinterpret_cast<const char*>(error_buf), returned)
                             : std::string_view("unknown or too long error? (latter unlikely)");
        // This is sure to fail at this point, we just needed the above lines
        // to generate context
        ASSERT(re, "regex could not be compiled",
               regex,
               ec,
               err_msg,
               error_at);
    }

    auto jit_status = pcre2_jit_compile(re, PCRE2_JIT_COMPLETE);
    ASSERT(jit_status == 0,
           "could not JIT lexer regular expression");

#ifdef C4_LEXER_ASSERT_RULES
    std::uint32_t groups{};
    pcre2_pattern_info(re, PCRE2_INFO_CAPTURECOUNT, &groups);
    ASSERT(groups <= max_capture_groups,
           "capture groups in lexer pattern must be less than or equal to max_capture_groups",
           regex);
    ASSERT(groups == groups_expected - 1,
           "capture groups in lexer pattern must equal that of the value given by Token::group_count",
           regex);

    std::uint32_t matches_empty{};
    pcre2_pattern_info(re, PCRE2_INFO_MATCHEMPTY, &matches_empty);
    ASSERT(matches_empty != 1,
           "PCRE2 believes there is a possibility this pattern matches empty string. This is not supported.",
           regex);
#endif
}

c4::p2::regex_rule::~regex_rule() noexcept {
    if (!_impl_handle) return;
    const auto re = static_cast<pcre2_code*>(_impl_handle);
    pcre2_code_free(re);
}

namespace {
    std::size_t
    utf8_strlen(std::string_view str) {
        auto str_view = str | una::views::utf8;
        return std::distance(str_view.begin(), str_view.end());
    }

    struct line_data {
        std::string_view remaining_buffer;
        std::size_t line_increment;
    };

    auto
    process_line_change(std::string_view match,
                        const char* buffer_end,
                        const std::string_view current_line) {
        auto line_start = current_line.data();
        std::size_t line_increment = 0;

        for (auto newline_at = match.find('\n');
             newline_at != std::string_view::npos;
             newline_at = match.find('\n')) {
            match = match.substr(newline_at + 1);
            line_start = match.data();
            ++line_increment;
        }

        return line_data{{line_start, buffer_end}, line_increment};
    }

    constexpr auto line_end_marks = std::string_view("\n\0", 2U);

    void
    update_position_for_multiline_match(c4::position& pos,
                                        const char* buffer_begin,
                                        std::string_view remaining_buffer,
                                        const std::size_t line_increment) {
        const auto it = std::ranges::find_first_of(remaining_buffer, line_end_marks);

        pos.line = remaining_buffer.substr(0, distance(begin(remaining_buffer), it));
        pos.col_number = buffer_begin - pos.line.data() + 1;
        pos.row_number += line_increment;
    }

    void
    update_position_for_single_line_match(c4::position& pos,
                                          const std::string_view match) {
        pos.col_number += utf8_strlen(match);
    }

    void
    update_position_for_match(c4::position& pos,
                              const char* begin, const char* end,
                              const std::string_view match) {
        if (const auto [buffer, line_increment] = process_line_change(match, end, pos.line);
            line_increment > 0) {
            update_position_for_multiline_match(pos, begin, buffer, line_increment);
        }
        else {
            update_position_for_single_line_match(pos, match);
        }
    }
}

void
c4::p2::regex_rule::offset_positions_newline(std::string_view match,
                                             const char*& begin, const char* end,
                                             position& pos) {
    DEBUG_ASSERT(!match.empty(), "empty string matched", pos.line);

    begin += match.size();
    update_position_for_match(pos, begin, end, match);

    DEBUG_ASSERT(begin <= end,
                 "begin must not advance after end",
                 pos.line,
                 match);
}

void
c4::p2::regex_rule::offset_positions_no_newline(const std::string_view match,
                                                const char*& begin, const char* end,
                                                position& pos) {
    DEBUG_ASSERT(match.find('\n') == match.npos,
                 "precondition: no_newline rule matched newline",
                 match);

    begin += match.size();
    pos.col_number += utf8_strlen(match);

    DEBUG_ASSERT(begin <= end,
                 "begin must not advance after end",
                 match);
}

std::optional<c4::position>
c4::p2::regex_rule::match_into(const char*& begin, const char* end,
                               const position& pos,
                               const regex_match_context& match_context,
                               std::span<std::string_view> matches) const {
    const auto match_data = static_cast<pcre2_match_data*>(match_context._impl);
    const auto re = static_cast<pcre2_code*>(_impl_handle);

    const auto res = pcre2_jit_match(
        re,
        reinterpret_cast<PCRE2_SPTR>(begin), end - begin,
        0, 0,
        match_data,
        nullptr
    );
    if (res < 0) return {};
    ASSERT(res >= 0,
           "match_context was not created with enough match slots");

    const auto ovector = pcre2_get_ovector_pointer(match_data);
    unsigned capture_idx = 0;
    for (auto i = 0; i < res; ++i) {
        matches[capture_idx++] = std::string_view(
            begin + ovector[2 * i],
            ovector[2 * i + 1] - ovector[2 * i]);
    }

    return pos.snapshot();
}
