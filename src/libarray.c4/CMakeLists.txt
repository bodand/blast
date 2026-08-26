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
# Originally created: 2026-08-09.
#
# src/libarray/CMakeLists.txt --
#   CMake source to provide libarray.

add_library(array-obj OBJECT
            inc/c4/array.h
            src/c4_array_new.c
            src/c4_array_ensure.c
            src/c4_array_ensure_more.c
            src/c4_array_external_typeid.c

            src/array/array_push.c
            src/array/new_array.c
            src/array/new_array_capacity.c
            src/array/new_array_sized.c
            )
target_link_libraries(array-obj PUBLIC c4rt3)
generate_warnings(array-obj PRIVATE)
target_include_directories(array-obj PUBLIC
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
                           $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

set(array-arch "${CMAKE_CURRENT_BINARY_DIR}/array.c4a")
add_custom_command(OUTPUT ${array-arch}
                   VERBATIM COMMAND c4ar -o "${array-arch}" $<TARGET_OBJECTS:array-obj>
                   DEPENDS c4ar array-obj
                   COMMAND_EXPAND_LISTS
                   COMMENT "Creating C4 library array.c4a...")

add_custom_target(libarray.c4a ALL DEPENDS "${array-arch}")

add_library(array INTERFACE)
add_dependencies(array libarray.c4a)

target_include_directories(array INTERFACE
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
                           $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_link_libraries(array INTERFACE
                      $<BUILD_INTERFACE:${array-arch}>
                      $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_LIBDIR}/array.c4a>)

install(FILES ${array-arch}
        DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(DIRECTORY inc/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(TARGETS array
        EXPORT c4std)
