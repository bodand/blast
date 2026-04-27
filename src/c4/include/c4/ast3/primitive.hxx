/* blAST project
 *
 * Copyright (c) 2026 András Bodor <bodand@pm.me>
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
 * Originally created: 2026-03-03.
 *
 * src/c4/include/c4/ast3/primitive --
 *   
 */
#ifndef BLAST_PRIMITIVE_HXX
#define BLAST_PRIMITIVE_HXX

#include <cstdint>
#include <string>
#include <variant>

#include <c4/ast3/ast_node.hxx>

namespace c4::ast3 {
struct primitive : ast_node {
    explicit primitive(std::int64_t value)
        : _value{value} { }

    explicit primitive(double value)
        : _value{value} { }

    explicit primitive(std::string_view value)
        : _value{value} { }

    [[nodiscard]] const std::string_view& str() const { return std::get<std::string_view>(_value); }
    [[nodiscard]] double fp() const { return std::get<double>(_value); }
    [[nodiscard]] std::int64_t i() const { return std::get<std::int64_t>(_value); }

private:
    std::variant<std::string_view, double, std::int64_t> _value;
};
}

#endif
