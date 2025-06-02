# demo project
#
# Copyright (c) 2025 András Bodor <bodand@proton.me>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# - Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
#
# - Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
#
# - Neither the name of the copyright holder nor the names of its contributors
#   may be used to endorse or promote products derived from this software
#   without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# Originally created: 2025-02-26.
#
# src/c4/CMakeLists.txt --
#

set(C4_LEXER_REGEX_BACKEND "pcre2")
add_library(c4
            include/c4/ast2/tags/source_positioned.hxx
            src/ast2/tags/source_positioned.cxx

            include/c4/ast2/symbol.hxx
            src/ast2/symbol.cxx
            include/c4/ast2/string_literal.hxx
            src/ast2/string_literal.cxx
            include/c4/ast2/integer_literal.hxx
            src/ast2/integer_literal.cxx
            include/c4/ast2/float_literal.hxx
            src/ast2/float_literal.cxx

            include/c4/p2/parser.hxx
            src/p2/parser.cxx

            include/c4/p2/lex/token_source.hxx
            src/p2/lex/token_source.cxx

            include/c4/p2/lex/tokens.hxx
            src/p2/lex/tokens.cxx

            include/c4/p2/lex/lexer.hxx
            src/p2/lex/lexer.cxx

            include/c4/p2/lex/regex_rule.hxx
            src/p2/lex/regex_rule.${C4_LEXER_REGEX_BACKEND}.cxx

            include/c4/diagnostic.hxx
            src/diagnostic.cxx

            src/ast2/let_expression.cxx
            include/c4/ast2/let_expression.hxx
            src/ast2/expression.cxx
            include/c4/ast2/expression.hxx
            include/c4/ast2/fn_call.hxx
            src/ast2/fn_call.cxx
            include/c4/p2/lex/string_rule.hxx
            src/p2/lex/string_rule.cxx
            include/c4/ast2/op_call.hxx
            src/ast2/op_call.cxx
            include/c4/ast2/block.hxx
            src/ast2/block.cxx
            include/c4/ast2/dynamic_call.hxx
            src/ast2/dynamic_call.cxx
            include/c4/ast2/tags/visitable.hxx
            src/ast2/tags/visitable.cxx
            include/c4/ast2/visitor/typeid.hxx
            src/ast2/visitor/typeid.cxx
            include/c4/ast2/visitor/visitor_base.hxx
            src/ast2/visitor/visitor_base.cxx
            include/c4/ast2/visitor/visitor.hxx
            src/ast2/visitor/visitor.cxx
            include/c4/ast_dumper.hxx
            src/ast_dumper.cxx
            include/c4/ast2/tags/evaluation_constness.hxx
            include/c4/ast2/tags/attributable.hxx
            src/ast2/tags/attributable.cxx
            include/c4/ast2/tags/referable.hxx
            src/ast2/tags/referable.cxx
            include/c4/ast2/ast_context.hxx
            src/ast2/ast_context.cxx
            include/c4/ast2/ast_node.hxx
            )

target_link_libraries(c4
                      PUBLIC
                      c4rt
                      PRIVATE
                      uni-algo::uni-algo
                      fmt::fmt-header-only
                      PCRE2::8BIT
                      libassert::assert
                      c4_llvm)
target_compile_options(c4 PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/Zc:preprocessor>)
target_compile_definitions(c4 PRIVATE
                           PCRE2_CODE_UNIT_WIDTH=8
                           C4_LEXER_ASSERT_RULES)
target_include_directories(c4
                           PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/private"
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
                           )

