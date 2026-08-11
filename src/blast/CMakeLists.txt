# blAST project
#
# Copyright (c) 2026 András Bodor <bodand@proton.me>
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
# Originally created: 2026-08-08.
#
# src/blast/CMakeLists.txt --
#

set(BLAST_DEFAULT_RESOURCE_DIR
    ${CMAKE_INSTALL_PREFIX}/share/blast/clang
    CACHE PATH "Default location for the clang resource directory")

add_library(blast STATIC
            src/blast/blast_build_ast_single.cxx
            src/blast/blast_init_.cxx
            src/blast/blast_match_ast.cxx

            src/ext-type.hxx
            src/blast_typeid.cxx

            src/default_resource_dir.in.hxx
            src/resource_dir.hxx
            src/resource_dir.cxx

            src/handler/diagnostic_handler.hxx
            src/handler/handler_base.hxx

            src/gc_box.hxx
            src/blast/blast_make_diagnostic.cxx
            )
configure_file(src/default_resource_dir.in.hxx
               default_resource_dir.hxx
               @ONLY)
target_include_directories(
        blast
        PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)

target_link_libraries(blast
                      PRIVATE
                      c4rt3
                      libarray
                      clangASTMatchers
                      clangTooling)
