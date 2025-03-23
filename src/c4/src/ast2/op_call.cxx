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
 * src/c4/include/c4/ast2/op_call --
 *   
 */


#include <c4/ast2/op_call.hxx>
#include <c4/ast2/expression.hxx>

#include <libassert/assert.hpp>

c4::ast2::binary_op_call::binary_op_call(const c4::position& position,
                                         const std::string_view file_source,
                                         const std::size_t length,
                                         const op_symbol& op,
                                         expression_ptr&& left,
                                         expression_ptr&& right)
    : source_positioned{position, file_source, length}
    , _op{op}
    , _left{std::move(left)}
    , _right{std::move(right)} {
    DEBUG_ASSERT(_op.arity() == 2,
                 "non-binary operator passed to binary expression",
                 _op);
    DEBUG_ASSERT(_left != nullptr,
                 "binary operator cannot have null subexpression");
    DEBUG_ASSERT(_right != nullptr,
                 "binary operator cannot have null subexpression");
}

c4::ast2::binary_op_call::binary_op_call(const binary_op_call& cp)
    : source_positioned(cp)
    , _op(cp._op)
    , _left(cp._left->clone().release())
    , _right(cp._right->clone().release()) { }

c4::ast2::binary_op_call&
c4::ast2::binary_op_call::operator=(const binary_op_call& cp) {
    DEBUG_ASSERT(&cp != this, "self-assignment is undefined");
    source_positioned::operator =(cp);
    _op = cp._op;
    _left = expression_ptr(cp._left->clone().release());
    _right = expression_ptr(cp._right->clone().release());
    return *this;
}

c4::ast2::unary_op_call::unary_op_call(const c4::position& position,
                                       const std::string_view file_source,
                                       const std::size_t length,
                                       const op_symbol& op,
                                       expression_ptr&& operand)
    : source_positioned{position, file_source, length}
    , _op{op}
    , _operand{std::move(operand)} {
    DEBUG_ASSERT(_op.arity() == 1,
                 "binary operator passed to unary expression",
                 _op);
    DEBUG_ASSERT(_operand != nullptr,
                 "unary operator cannot have null subexpression");
}

c4::ast2::unary_op_call::unary_op_call(const unary_op_call& cp)
    : source_positioned{cp}
    , _op{cp._op}
    , _operand{cp._operand->clone().release()} { }

c4::ast2::unary_op_call&
c4::ast2::unary_op_call::operator=(const unary_op_call& cp) {
    DEBUG_ASSERT(&cp != this, "self-assignment is undefined");
    source_positioned::operator=(cp);
    _op = cp._op;
    _operand = expression_ptr(cp._operand->clone().release());
    return *this;
}
