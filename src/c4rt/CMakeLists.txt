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
# Originally created: 2025-03-22.
#
# src/c4rt/CMakeLists.txt --
#   The library for the C4 runtime. Contains various utilities required by the
#   runtime.

project(c4rt C CXX)

set(C4RT_SOURCES
    include/c4rt/api.h
    include/c4rt/string.h
    src/string.cxx
    private/c4rt-impl/string_pool.hxx
    include/c4rt/datum.h
    src/datum.cxx
    )

configure_file(src/dll-config.h.in dll-config.h @ONLY)

add_library(c4rt-static STATIC ${C4RT_SOURCES})
add_library(c4rt SHARED ${C4RT_SOURCES}
            src/DllMain.cxx
            ${CMAKE_CURRENT_BINARY_DIR}/dll-config.h)

target_compile_options(c4rt-static PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/Zc:preprocessor>)
target_compile_options(c4rt PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/Zc:preprocessor>)

target_compile_definitions(c4rt-static PUBLIC C4RT_STATIC)
target_compile_definitions(c4rt PRIVATE $<BUILD_INTERFACE:C4RT_EXPORTS>)

target_include_directories(c4rt-static
                           PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
target_include_directories(c4rt
                           PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                           PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
                           )

target_link_libraries(c4rt-static PRIVATE libassert::assert)
target_link_libraries(c4rt PRIVATE libassert::assert)
if (BLAST_USE_MIMALLOC)
    target_link_libraries(c4rt PRIVATE $<IF:$<TARGET_EXISTS:mimalloc-static>,mimalloc-static,mimalloc>)
    if (WIN32 AND NOT TARGET mimalloc-static)
        add_custom_command(TARGET c4rt POST_BUILD
                           COMMAND "${CMAKE_COMMAND}" -E copy
                           $<TARGET_FILE:mimalloc>
                           $<TARGET_FILE_DIR:c4rt>)
    endif ()
endif ()
