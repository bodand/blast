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

if (UNIX)
    set(C4_DEFAULT_SEARCH_PATHS_RAW
        "\"${CMAKE_INSTALL_PREFIX}/lib\""
        "\"${CMAKE_INSTALL_PREFIX}/lib64\"")
    string(REPLACE ";" ", " C4_DEFAULT_SEARCH_PATHS "${C4_DEFAULT_SEARCH_PATHS_RAW}")
else ()
    message(SEND_ERROR "C4 is currently not configurable on your platform.
 We need to figure out a default search path that makes sense.")
endif ()

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
            include/c4/ast2/fwd.hxx

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

            # Tags
            include/c4/tags/source_positioned.hxx
            src/tags/source_positioned.cxx

            include/c4/tags/visitable.hxx
            src/tags/visitable.cxx

            include/c4/tags/literal_from_token.hxx
            src/tags/literal_from_token.cxx

            include/c4/tags/attributable.hxx
            src/tags/attributable.cxx

            include/c4/tags/referable.hxx
            src/tags/referable.cxx

            include/c4/tags/tailable.hxx

            # Visitor
            include/c4/visitor/typeid.hxx
            src/visitor/typeid.cxx

            include/c4/visitor/visitor_base.hxx
            src/visitor/visitor_base.cxx

            include/c4/visitor/visitor.hxx
            src/visitor/visitor.cxx

            # Misc
            include/c4/diagnostic.hxx
            src/diagnostic.cxx

            include/c4/ast_dumper.hxx
            src/ast_dumper.cxx
            include/c4/fmt.hxx

            include/c4/p2/parser-aux.hxx
            src/p2/parser_utils.hxx

            include/c4/archive_file.hxx
            src/archive_file/archive_file.cxx
            src/archive_file/lex.cxx

            include/c4/p2/a/lexer.hxx
            src/p2/a/lexer/lexer.cxx
            src/p2/a/lexer/next.cxx

            include/c4/p2/archive_parser.hxx
            src/p2/archive_parser.hxx/archive_parser.cxx
            src/p2/archive_parser.hxx/parse.cxx

            include/c4/p2/included_parser.hxx
            src/p2/included_parser/included_parser.cxx
            src/p2/included_parser/next_relevant.cxx
            src/p2/included_parser/parse_associativity_indicator.cxx
            src/p2/included_parser/parse_expression.cxx
            src/p2/included_parser/parse_final_expression.cxx
            src/p2/included_parser/parse_fn_let.cxx
            src/p2/included_parser/parse_global_let.cxx
            src/p2/included_parser/parse_let_expression.cxx
            src/p2/included_parser/parse_op_let.cxx
            src/p2/included_parser/parse_op_symbol.cxx
            src/p2/included_parser/parse_operator_precedence.cxx
            src/p2/included_parser/parse_precedence.cxx
            src/p2/included_parser/parse_symbol.cxx
            src/p2/included_parser/parse_use_expression.cxx
            src/p2/included_parser/throw_away_expression.cxx

            include/c4/p2/named_source.hxx

            src/p2/source_resolver/open_archive.cxx
            src/p2/source_resolver/open_source.cxx
            src/p2/source_resolver/source_resolver.cxx

            include/c4/p2/symbol_table.hxx
            src/p2/symbol_table/declare.cxx
            src/p2/symbol_table/enter_scope.cxx
            src/p2/symbol_table/find_operator.cxx
            src/p2/symbol_table/find_scoped_symbol.cxx
            src/p2/symbol_table/find_scoped_symbol_with_arity.cxx
            src/p2/symbol_table/leave_scope.cxx

            include/c4/p2/source_resolver.hxx

            include/c4/source_file.hxx
            src/source_file/begin.cxx
            src/source_file/end.cxx
            src/source_file/lex.cxx
            src/source_file/resolve_source_use.cxx
            src/source_file/source_file.cxx
            include/c4/cc_attr.hxx
            )
configure_file(src/p2/source_resolver/source-config.in.h
               source-config.h
               @ONLY)

target_link_libraries(c4
                      PUBLIC
                      fmt::fmt
                      PRIVATE
                      LIEF::LIEF
                      uni-algo::uni-algo
                      PCRE2::8BIT
                      libassert::assert
                      )
generate_warnings(c4 PRIVATE)
generate_lto(c4 PRIVATE)
target_compile_options(c4 PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/Zc:preprocessor>)
target_compile_definitions(c4 PRIVATE
                           PCRE2_CODE_UNIT_WIDTH=8
                           C4_LEXER_ASSERT_RULES)
target_include_directories(c4
                           PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
                           )

