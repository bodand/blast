# blAST project
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
# Originally created: 2025-03-26.
#
# src/c4c/CMakeLists.txt --
#   CMake project for the C4 Compiler (c4c)s

add_executable(c4i src/c4i.cxx
               include/gc_registrar.hxx
               src/gc_registrar.cxx)

target_link_libraries(c4i PRIVATE
                      c4
                      c4rt2c
                      c4_llvm
                      bfg::lyra
                      libassert::assert
                      gc
                      )
target_include_directories(c4i PRIVATE
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

if (WIN32)
    add_custom_command(
            TARGET c4i POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:ZLIB::ZLIB>
            $<TARGET_FILE:BDWgc::gc>
            $<TARGET_FILE_DIR:c4i>
            )
endif ()
if (MSVC AND BLAST_ENABLE_SANITIZERS)
    cmake_path(GET CMAKE_CXX_COMPILER PARENT_PATH msvc_bindir)
    add_custom_command(
            TARGET blast POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${msvc_bindir}/clang_rt.asan_dynamic-${BLAST_ARCH}.dll"
            "${msvc_bindir}/clang_rt.asan_dbg_dynamic-${BLAST_ARCH}.dll"
            "${msvc_bindir}/clang_rt.asan_dynamic-${BLAST_ARCH}.pdb"
            "${msvc_bindir}/clang_rt.asan_dbg_dynamic-${BLAST_ARCH}.pdb"
            $<TARGET_FILE_DIR:c4i>
            )
endif ()
