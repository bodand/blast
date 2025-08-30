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
            # Parser
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

            include/c4/p2/lex/string_rule.hxx
            src/p2/lex/string_rule.cxx

            src/p2/lex/match_helper.hxx
            src/p2/lex/match_helper.cxx

            # AST
            include/c4/ast2/symbol.hxx
            src/ast2/symbol.cxx

            include/c4/ast2/string_literal.hxx
            src/ast2/string_literal.cxx

            include/c4/ast2/integer_literal.hxx
            src/ast2/integer_literal.cxx

            include/c4/ast2/float_literal.hxx
            src/ast2/float_literal.cxx

            include/c4/ast2/let_expression.hxx
            src/ast2/let_expression.cxx

            include/c4/ast2/expression.hxx
            src/ast2/expression.cxx

            include/c4/ast2/fn_call.hxx
            src/ast2/fn_call.cxx

            include/c4/ast2/op_call.hxx
            src/ast2/op_call.cxx

            include/c4/ast2/block.hxx
            src/ast2/block.cxx

            include/c4/ast2/dynamic_call.hxx
            src/ast2/dynamic_call.cxx

            include/c4/ast2/ast_node.hxx
            # no source

            include/c4/ast2/ast_context.hxx
            src/ast2/ast_context.cxx

            # FFM
            include/c4/ffm/fwd.hxx
            include/c4/ffm.hxx

            include/c4/ffm/block_argument.hxx
            src/ffm/block_argument.cxx

            include/c4/ffm/context_type.hxx
            src/ffm/context_type.cxx

            include/c4/ffm/expression.hxx
            src/ffm/expression.cxx

            include/c4/ffm/ffm_context.hxx
            src/ffm/ffm_context.cxx

            include/c4/ffm/ffm_node.hxx
            src/ffm/ffm_node.cxx

            include/c4/ffm/function.hxx
            src/ffm/function.cxx

            include/c4/ffm/function_call.hxx
            src/ffm/function_call.cxx

            include/c4/ffm/function_declaration.hxx
            src/ffm/function_declaration.cxx

            include/c4/ffm/function_definition.hxx
            src/ffm/function_definition.cxx

            include/c4/ffm/literal.hxx
            src/ffm/literal.cxx

            include/c4/ffm/local.hxx
            src/ffm/local.cxx

            include/c4/ffm/symbol.hxx
            src/ffm/symbol.cxx

            include/c4/ffm/unpack.hxx
            src/ffm/unpack.cxx

            # Tags
            include/c4/tags/source_positioned.hxx
            src/tags/source_positioned.cxx

            include/c4/tags/visitable.hxx
            src/tags/visitable.cxx

            include/c4/tags/evaluation_constness.hxx

            include/c4/tags/literal_from_token.hxx
            src/tags/literal_from_token.cxx

            include/c4/tags/attributable.hxx
            src/tags/attributable.cxx

            include/c4/tags/referable.hxx
            src/tags/referable.cxx

            # Visitor
            include/c4/visitor/typeid.hxx
            src/visitor/typeid.cxx

            include/c4/visitor/visitor_base.hxx
            src/visitor/visitor_base.cxx

            include/c4/visitor/visitor.hxx
            src/visitor/visitor.cxx

            # ffm utils
            include/c4/ffm_dumper.hxx
            src/ffm_dumper.cxx

            include/c4/ffm_mapper.hxx
            src/ffm_mapper.cxx

            src/ffm/attributes.cxx
            src/ffm/attributes.hxx

            # Misc
            include/c4/diagnostic.hxx
            src/diagnostic.cxx

            include/c4/ast_dumper.hxx
            src/ast_dumper.cxx
            include/c4/ffm.hxx
            include/c4/fmt.hxx
            )

target_link_libraries(c4
                      PUBLIC
                      c4rt2
                      fmt::fmt
                      PRIVATE
                      internal-warnings
                      internal-lto
                      uni-algo::uni-algo
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

