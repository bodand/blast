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
#include <ranges>

#include <c4/diagnostic.hxx>
#include <c4/p2/lex/token_source.hxx>

#include <fmt/format.h>
#ifdef __clang__
// There is some fuckery with clang and FMT_STRING - ignore it
#undef FMT_STRING
#define FMT_STRING(x) x
#endif
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

	decltype(auto)
	styled_as_type(const std::string_view& str,
	               const c4::source_diagnostic::diag_type type) {
		return styled(str, fg(color_from_type(type)));
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
		return static_cast<std::size_t>(std::distance(str_view.begin(), str_view.end()));
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
	// XXX allocating on what is basically a subrange calculation, truly
	//  the pinnacle of software engineering
	auto leading_utf_str = expanded_range
	                       | una::views::utf8
	                       | una::views::take(col_number - 1)
	                       | una::ranges::to_utf8<std::string>();
	const auto range_begin = leading_utf_str.size();
	const auto last_ln_idx = expanded_range.rfind('\n');
	if (last_ln_idx == std::string_view::npos) {
		const auto utf_str = expanded_range
		                     | una::views::utf8
		                     | una::views::drop(col_number - 1)
		                     | una::views::take(col_number_end - col_number + 1)
		                     | una::ranges::to_utf8<std::string>();
		return expanded_range.substr(range_begin, utf_str.size());
	}
	const auto last_line_start = last_ln_idx + 1;
	const auto last_line_range_end = last_line_start + col_number_end;
	return expanded_range.substr(range_begin, last_line_range_end - range_begin);
}

c4::diagnostics_bundle::~diagnostics_bundle() noexcept {
	_engine.emit(*this);
}

c4::diagnostics_bundle&&
c4::diagnostics_bundle::emplace_diagnostic(source_diagnostic::diag_type type,
                                           const position& position,
                                           const fmt::string_view diagnostic,
                                           const fmt::format_args args) && {
	if (!_skip_next) {
		_tail.emplace_back(
			type,
			fmt::vformat(diagnostic, args),
			position
		);
		_skip_next = false;
	}
	return std::move(*this);
}

c4::diagnostics_bundle&&
c4::diagnostics_bundle::emplace_diagnostic(source_diagnostic::diag_type type,
                                           position&& position,
                                           const fmt::string_view diagnostic,
                                           const fmt::format_args args) && {
	if (!_skip_next) {
		_tail.emplace_back(
			type,
			fmt::vformat(diagnostic, args),
			position
		);
		_skip_next = false;
	}
	return std::move(*this);
}

void
c4::diagnostics_engine::emit(const diagnostics_bundle& bundle) const {
	fmt::print(_output, "{}", bundle._head);
	for (const auto& diag : bundle._tail) {
		fmt::print(_output, "{}", diag);
	}
}

fmt::context::iterator
fmt::formatter<c4::source_diagnostic>::format(const c4::source_diagnostic& diag, format_context& ctx) const {
	// XXX UTF-8 support is as hacky as could be... to noone's surprise, really
	const auto& pos = diag.position();
	const auto diagnostic_type = diag.diagnostic_type();
	const auto diagnostic_string = diag.diagnostic();

	const auto range = pos.range();

	// Using row_number_end as it is greater than or equal to row_number, so if
	// that first nicely with a preceding space, so those the other and all betwixt
	constexpr static auto line_number_padding = 3;
	const auto line_number_size =
			static_cast<unsigned long long>(std::floor(std::log10(pos.row_number_end))) + 1
			+ line_number_padding;

	const auto leading_utf_str = pos.expanded_range
	                             | una::views::utf8
	                             | una::views::take(pos.col_number - 1)
	                             | una::ranges::to_utf8<std::string>();

	const auto first_line_skip = leading_utf_str.size();
	const auto last_line_take = pos.col_number_end;

	const auto caret_leading_spaces = utf8_strlen(leading_utf_str);
	const auto head_line_str = caret_leading_spaces == 0
		                           ? fmt::format(
			                           "{{0:>{0}}} | {{1}}{{2}}{{3}}\n{{4:>{0}}} | {{6}}{{7}}\n",
			                           line_number_size
		                           )
		                           : fmt::format(
			                           "{{0:>{0}}} | {{1}}{{2}}{{3}}\n{{4:>{0}}} | {{5:>{1}}}{{6}}{{7}}\n",
			                           line_number_size,
			                           caret_leading_spaces
		                           );
	const auto head_line_fmt = fmt::runtime(head_line_str);
	const auto body_line_str = fmt::format(
		"{{:>{0}}} | {{}}\n{{:>{0}}} | {{}}\n",
		line_number_size
	);
	const auto body_line_fmt = fmt::runtime(body_line_str);
	const auto tail_line_str = fmt::format(
		"{{:>{0}}} | {{}}{{}}\n{{:>{0}}} | {{}}\n",
		line_number_size
	);
	const auto tail_line_fmt = fmt::runtime(tail_line_str);

	auto ret = ctx.out();
	auto line_number = pos.row_number;

	for (const auto line : pos.expanded_range | std::views::split('\n')) {
		if (line.empty()) continue;
		auto line_str = std::string_view(line);

		if (line_number == pos.row_number) {
			ret = fmt::format_to(
				ret, "{}:{}: {} {}\n", pos.filename(), line_number,
				prefix_from_type(diagnostic_type), diagnostic_string
			);

			const auto first_line_range = range.substr(0, range.find('\n'));

			const auto utf_len = utf8_strlen(first_line_range) - 1; // -1 for ^
			ret = fmt::format_to(
				ret, head_line_fmt,
				line_number,
				line_str.substr(0, first_line_skip),
				styled_as_type(first_line_range, diagnostic_type),
				line_str.substr(first_line_skip + first_line_range.size()),
				' ',
				' ',
				styled_as_type("^", diagnostic_type),
				styled_as_type(std::string(utf_len, '~'), diagnostic_type)
			);
		}
		else if (line_number == pos.row_number_end) {
			const auto first_line_range = range.substr(range.rfind('\n') + 1);
			const auto utf_len = utf8_strlen(first_line_range);

			ret = fmt::format_to(
				ret, tail_line_fmt,
				line_number,
				styled_as_type(line_str.substr(0, last_line_take), diagnostic_type),
				line_str.substr(last_line_take),
				' ',
				styled_as_type(std::string(utf_len, '~'), diagnostic_type)
			);
		}
		else {
			ret = fmt::format_to(
				ret, body_line_fmt,
				line_number,
				styled_as_type(line_str, diagnostic_type),
				' ',
				styled_as_type(std::string(utf8_strlen(line_str), '~'), diagnostic_type)
			);
		}
		++line_number;
	}

	return ret;
}

c4::position
c4::position::pseudo_position() {
	static p2::token_source pseudo_source;
	return {"", 1, 1, 1, 1, "", &pseudo_source};
}
