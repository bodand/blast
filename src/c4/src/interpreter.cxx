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
 * src/c4/src/interpreter --
 *   
 */

#include <c4/interpreter.hxx>
#include <c4/parser.hxx>

namespace c4p = ::c4::parser;
namespace c4a = ::c4::ast;

bool
c4::interpreter::parse(const std::string_view str) {
    // TODO this is horribly inefficient
    return parse(std::string(str));
}

bool
c4::interpreter::parse(const std::string& str) {
    auto begin = str.cbegin();
    const auto end = str.cend();

    using boost::spirit::x3::with;

    c4p::position_cache position_cache{begin, end};
    c4p::error_handler eh{begin, end, _error_stream, "<string>"};
    const auto parser =
            with<c4p::position_cache_tag>(std::ref(position_cache))[
                with<c4p::error_handler_tag>(std::ref(eh))[
                    with<c4p::symbol_scope_tag>(std::ref(_global_scope))[
                        *c4p::expression()
                    ]]];

    const bool r = phrase_parse(begin, end,
                                parser,
                                boost::spirit::x3::ascii::space,
                                _expressions);
    return r && begin == end;
}

int
c4::interpreter::exec() const {
    if (_expressions.empty()) return 0;

    value last{0ll};
    for (const auto& expr: _expressions) {
        last = expr.evaluate(_evaluation_stack);
    }
    return static_cast<int>(last.coerce_to_int() & ~(~std::int64_t{} << (sizeof(int) * CHAR_BIT)));
    //                                             ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    //                                             |~~~~~~~~~~~~~~~~~^~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Bitshift magic:                             | ^~~~~~~~~~~~~~~ |  ^~~~~~~~~~~~~~~~~~~~~~~~
    //                                             | |               |  |
    //                                             | |               |  |- The number of bits in an int
    //                                             | |               |- Shift in equal zeros bit int bits to
    //                                             | |                  make equal a bitmask that is full 0
    //                                             | |                  for the bits of an int and 1 for other
    //                                             | |- Full 1 bits for size of the input integer type
    //                                             |- Reverse result of shit, making a bitmask that
    //                                                only gets us the bits of an int
}
