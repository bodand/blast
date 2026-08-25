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
 * src/c4/src/p2/lex/match_helper --
 *   
 */

#include "match_helper.hxx"

#include <c4/diagnostic.hxx>

namespace {
	constexpr auto line_end_marks = std::string_view("\n\0", 2U);

	line_data
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

		return {{line_start, buffer_end}, line_increment};
	}

	void
	update_position_for_multiline_match(c4::position& pos,
	                                    const char* buffer_begin,
	                                    std::string_view remaining_buffer,
	                                    const std::size_t line_increment) {
		const auto line_end_it = std::ranges::find_first_of(remaining_buffer, line_end_marks);
		const auto line_sz = static_cast<size_t>(std::ranges::distance(begin(remaining_buffer), line_end_it));

		DEBUG_ASSERT(buffer_begin >= pos.line.data(), "negative length match");

		pos.line = remaining_buffer.substr(0, line_sz);
		pos.col_number = static_cast<std::size_t>(buffer_begin - pos.line.data()) + 1;
		pos.row_number += line_increment;
	}

	void
	update_position_for_single_line_match(c4::position& pos, const std::string_view match) {
		pos.col_number += utf8_strlen(match);
	}
}

void
update_position_for_match(c4::position& pos, const char* begin, const char* end, const std::string_view match) {
	// CLEVER CODE WARNING: rfind returns npos if it did not find the newline
	// meaning we have only a single line range so for process_line_change
	// that needs the last line of the current range, we need to pass the
	// whole expanded_range, otherwise we need to substr starting from the
	// return of rfind *+ 1*, so we don't take the newline itself.
	// Since npos is a maximum value for an UNSIGNED type, adding 1 to it
	// wraps around to zero, which when used in substr returns the whole
	// string just as we need it without complex branching code or non-const
	// variables
	const auto last_newline_idx = pos.expanded_range.rfind('\n') + 1;
	const auto last_newline = pos.expanded_range.substr(last_newline_idx);

	if (const auto [buffer, line_increment] = process_line_change(match, end, last_newline);
		line_increment > 0) {
		update_position_for_multiline_match(pos, begin, buffer, line_increment);
	}
	else {
		update_position_for_single_line_match(pos, match);
	}
}
