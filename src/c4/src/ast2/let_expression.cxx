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
 * src/c4/src/ast2/let_expression --
 *   
 */

#include <c4/ast2/let_expression.hxx>
#include <c4/ast2/expression.hxx>

#include <libassert/assert.hpp>

c4::ast2::let_expression::let_expression(const c4::position& position,
                                         const std::string_view file_source,
                                         const std::size_t length,
                                         ast2::symbol symbol,
                                         expression&& expr)
    : source_positioned{position, file_source, length}
    , _symbol{std::move(symbol)}
    , _value{expression_ptr(new expression(std::move(expr)))} { }

c4::ast2::let_expression::let_expression(const let_expression& cp)
    : source_positioned{cp}
    , _symbol{cp._symbol}
    , _value{cp._value->clone().release()} { }

c4::ast2::let_expression&
c4::ast2::let_expression::operator=(const let_expression& cp) {
    DEBUG_ASSERT(&cp != this, "self-assignment is undefined");

    source_positioned::operator=(cp);
    _symbol = cp._symbol;
    _value = expression_ptr(cp._value->clone().release());
    return *this;
}

const c4::ast2::expression&
c4::ast2::let_expression::value() const {
    DEBUG_ASSERT(_value != nullptr, "let-expression's value is not set", _symbol);
    return *_value;
}

c4::ast2::let_expression::let_expression(const c4::position& position,
                                         const std::string_view file_source,
                                         const std::size_t length,
                                         ast2::symbol symbol,
                                         expression_ptr&& expr)
    : source_positioned{position, file_source, length}
    , _symbol{std::move(symbol)}
    , _value{std::move(expr)} { }
