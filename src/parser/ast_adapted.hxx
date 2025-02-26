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
 * Originally created: 2025-02-17.
 *
 * src/parser/private/ast_adapted --
 *   
 */
#ifndef DEMO_AST_ADAPTED_HXX
#define DEMO_AST_ADAPTED_HXX

#include <boost/fusion/include/adapt_struct.hpp>
#include "ast.hxx"

BOOST_FUSION_ADAPT_STRUCT(
        demo::ast::let_expression,
        symbol,
        expr
)

BOOST_FUSION_ADAPT_STRUCT(
        demo::ast::symbol_expression,
        symbol,
        arguments
)

BOOST_FUSION_ADAPT_STRUCT(
        demo::ast::block_parameters,
        parameters
)

BOOST_FUSION_ADAPT_STRUCT(
        demo::ast::block_expression,
        parameters,
        expressions
)

BOOST_FUSION_ADAPT_STRUCT(
        demo::ast::script,
        expressions
)

BOOST_FUSION_ADAPT_STRUCT(
        demo::ast::symbol,
        name,
        arity
)

#endif
