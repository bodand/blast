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
 * src/c4/src/p2/lex/string_rule --
 *   
 */

#include <c4/p2/lex/string_rule.hxx>
#include <c4/diagnostic.hxx>

#include <libassert/assert.hpp>

#include "match_helper.hxx"

void
c4::p2::string_rule::update_matched_position(const char*& data, const char* end,
                                             position& pos,
                                             position& matched_at) const {
    if (_match_newline) {
        offset_positions_newline(data, end, pos);
    }
    else {
        offset_positions_no_newline(data, end, pos);
    }
    // pos now points to next character to be read
    matched_at.row_number_end = pos.row_number;
    matched_at.col_number_end = pos.col_number - 1;

    const auto range_start = matched_at.line.data();
    const auto range_end = pos.line.data() + pos.line.size();
    matched_at.expanded_range = std::string_view{range_start, range_end};
}

void
c4::p2::string_rule::offset_positions_newline(const char*& begin, const char* end,
                                              position& pos) const {
    DEBUG_ASSERT(_str.find('\n') != _str.npos,
                 "precondition: pos update with newline did not match newline",
                 _str);

    begin += _str.size();
    update_position_for_match(pos, begin, end, _str);

    DEBUG_ASSERT(begin <= end,
                 "begin must not advance after end",
                 pos.expanded_range,
                 _str);
}

void
c4::p2::string_rule::offset_positions_no_newline(const char*& begin, const char* end,
                                                 position& pos) const {
    DEBUG_ASSERT(_str.find('\n') == _str.npos,
                 "precondition: no_newline rule matched newline",
                 _str);

    begin += _str.size();
    pos.col_number += utf8_strlen(_str);

    DEBUG_ASSERT(begin <= end,
                 "begin must not advance after end",
                 _str);
}
