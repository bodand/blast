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
 * Originally created: 2025-02-02.
 *
 * src/c4/include/c4/ast/op_call --
 *   
 */
#ifndef AST_OP_CALL_FUSION_HXX
#define AST_OP_CALL_FUSION_HXX

#include <boost/fusion/include/adapt_struct.hpp>
#include <c4/ast/op_call.hxx>

BOOST_FUSION_ADAPT_STRUCT(typename c4::ast::precedence_op_expr<9>::op_chain,
                          op,
                          precedence)
BOOST_FUSION_ADAPT_STRUCT(typename c4::ast::precedence_op_expr<9>,
                          next,
                          ops)

BOOST_FUSION_ADAPT_STRUCT(typename c4::ast::precedence_op_expr<8>::op_chain,
                          op,
                          precedence)
BOOST_FUSION_ADAPT_STRUCT(typename c4::ast::precedence_op_expr<8>,
                          next,
                          ops)

BOOST_FUSION_ADAPT_STRUCT(typename c4::ast::precedence_op_expr<7>::op_chain,
                          op,
                          precedence)
BOOST_FUSION_ADAPT_STRUCT(typename c4::ast::precedence_op_expr<7>,
                          next,
                          ops)

BOOST_FUSION_ADAPT_STRUCT(typename c4::ast::precedence_op_expr<6>::op_chain,
                          op,
                          precedence)
BOOST_FUSION_ADAPT_STRUCT(typename c4::ast::precedence_op_expr<6>,
                          next,
                          ops)

BOOST_FUSION_ADAPT_STRUCT(typename c4::ast::precedence_op_expr<5>::op_chain,
                          op,
                          precedence)
BOOST_FUSION_ADAPT_STRUCT(typename c4::ast::precedence_op_expr<5>,
                          next,
                          ops)

BOOST_FUSION_ADAPT_STRUCT(typename c4::ast::precedence_op_expr<4>::op_chain,
                          op,
                          precedence)
BOOST_FUSION_ADAPT_STRUCT(typename c4::ast::precedence_op_expr<4>,
                          next,
                          ops)

BOOST_FUSION_ADAPT_STRUCT(typename c4::ast::precedence_op_expr<3>::op_chain,
                          op,
                          precedence)
BOOST_FUSION_ADAPT_STRUCT(typename c4::ast::precedence_op_expr<3>,
                          next,
                          ops)

BOOST_FUSION_ADAPT_STRUCT(typename c4::ast::precedence_op_expr<2>::op_chain,
                          op,
                          precedence)
BOOST_FUSION_ADAPT_STRUCT(typename c4::ast::precedence_op_expr<2>,
                          next,
                          ops)

BOOST_FUSION_ADAPT_STRUCT(typename c4::ast::precedence_op_expr<1>::op_chain,
                          op,
                          precedence)
BOOST_FUSION_ADAPT_STRUCT(typename c4::ast::precedence_op_expr<1>,
                          next,
                          ops)

BOOST_FUSION_ADAPT_STRUCT(c4::ast::op_call,
                         expression)

#endif
