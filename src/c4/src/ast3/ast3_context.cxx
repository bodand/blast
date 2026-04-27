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
 * src/c4/src/ast3/ast3_context --
 *   
 */

#include <c4/ast3/ast_context.hxx>
#include <c4/ast3/primitive.hxx>

c4::ast3::primitive*
c4::ast3::ast_context::build_primitive(double value) {
    auto primitive_obj = std::make_unique<primitive>(value);
    const auto pointer = primitive_obj.get();
    _nodes.emplace_back(std::move(primitive_obj));
    return pointer;
}

c4::ast3::primitive*
c4::ast3::ast_context::build_primitive(std::int64_t value) {
    auto primitive_obj = std::make_unique<primitive>(value);
    const auto pointer = primitive_obj.get();
    _nodes.emplace_back(std::move(primitive_obj));
    return pointer;
}

c4::ast3::primitive*
c4::ast3::ast_context::build_primitive(std::string_view value) {
    auto primitive_obj = std::make_unique<primitive>(value);
    const auto pointer = primitive_obj.get();
    _nodes.emplace_back(std::move(primitive_obj));
    return pointer;
}
