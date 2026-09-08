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
#   CMake project for the C4 JIT interpreter (c4i)

include(CheckLinkerFlag)

check_linker_flag(CXX "-Wl,--export-dynamic" HAVE_LD_EXPORT_DYNAMIC)
check_linker_flag(CXX "-Wl,-rdynamic" HAVE_LD_RDYNAMIC)

set(ld_export_flag)
if (HAVE_LD_EXPORT_DYNAMIC)
    set(ld_export_flag "-Wl,--export-dynamic")
elseif (HAVE_LD_RDYNAMIC)
    set(ld_export_flag "-Wl,-rdynamic")
else ()
    message(WARNING "No known way to reexport linked symbols from executable.
The JIT interpreter may break.")
endif ()

add_executable(c4i
               src/c4i.cxx
               include/c4i/gc_registrar.hxx
               src/c4i/gc_registrar.cxx
               include/c4i/lazy_codegen.hxx
               src/c4i/lazy_codegen/add_parsed.cxx
               include/c4i/dependee.hxx
               src/c4i/dependee/source_dependee/emit.cxx)

target_link_libraries(c4i PRIVATE
                      c4
                      c4rt2c
                      c4_llvm
                      sgetopt
                      libassert::assert
                      gc
                      )
generate_lto(c4i PRIVATE)
generate_warnings(c4i PRIVATE)
generate_sanitizers(c4i)
target_link_options(c4i PRIVATE ${ld_export_flag})
target_include_directories(c4i PRIVATE
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

install(TARGETS c4i
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
