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

macro(add_ast_elem tgt name)
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/include/c4/ast/${name}_fusion.hxx")
        set(fusion "include/c4/ast/${name}_fusion.hxx")
    endif ()
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/ast/${name}.cxx")
        set(src "src/ast/${name}.cxx")
    endif ()
    target_sources(${tgt}
                   # AST node
                   PUBLIC include/c4/ast/${name}.hxx
                   PUBLIC "${fusion}"
                   # Parser API
                   PUBLIC include/c4/parser/${name}.hxx
                   # Parser implementation & instantiation
                   PRIVATE "${src}"
                   PRIVATE src/parser/${name}.cxx
                   PRIVATE private/c4/parser/${name}_def.hxx
                   )
endmacro()

add_executable(blast
               src/blast.cxx

               include/c4/ast.hxx
               include/c4/ast_fwd.hxx

               include/c4/symbol.hxx
               src/symbol.cxx

               include/c4/block.hxx
               src/block.cxx

               include/c4/evaluation_stack.hxx
               src/evaluation_stack.cxx

               include/c4/value.hxx
               src/value.cxx

               include/c4/parser/config.hxx
               private/c4/parser/position_annotator.hxx
               private/c4/parser/error_handler_callback.hxx
               private/c4/parser/valid_symbol_parser.hxx
               include/c4/ast/symbol_scope.hxx
               src/ast/symbol_scope.cxx
               )

add_ast_elem(blast symbol)
add_ast_elem(blast fundamental_scalar)
add_ast_elem(blast block_expression)
add_ast_elem(blast expression)
add_ast_elem(blast fn_call)
add_ast_elem(blast op_call)
add_ast_elem(blast let_expression)

target_link_libraries(blast PRIVATE
                      Boost::spirit
                      mio::mio_min_winapi)
target_compile_definitions(blast PRIVATE SPIRIT_DEBUG)
target_include_directories(blast
                           PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/private"
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
                           )
