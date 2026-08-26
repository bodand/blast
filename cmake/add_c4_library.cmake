# blAST project
#
# Copyright (c) 2026, András Bodor <bodand@proton.me>
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
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
# cmake/add_c4_library.cmake --

function(add_c4_library)
    set(opts "")
    set(oneFlags "TARGET")
    set(manyFlags "FLAGS;C_SOURCES;C4_SOURCES;C4_DEPENDS")
    cmake_parse_arguments(PARSE_ARGV 0 cc4 "${opts}" "${oneFlags}" "${manyFlags}")

    set(objects)
    foreach (src IN LISTS cc4_C4_SOURCES)
        set(obj "${CMAKE_CURRENT_BINARY_DIR}/${src}.o")
        list(APPEND objects "${obj}")
        add_custom_command(OUTPUT "${obj}"
                           VERBATIM COMMAND
                           c4c
                           $<$<CONFIG:Debug>:-O0>
                           $<$<CONFIG:MinSizeRel>:-Os>
                           $<$<CONFIG:Release>:-O3>
                           # Disable standard load paths: if the user has these
                           # libraries installed, it would find those instead
                           -I- ${cc4_FLAGS}
                           -o "${obj}" "${CMAKE_CURRENT_SOURCE_DIR}/${src}"
                           DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${src}" c4c ${cc4_C4_DEPENDS}
                           COMMAND_EXPAND_LISTS
                           COMMENT "Building C4 object ${obj}...")
    endforeach ()
    add_custom_target(${cc4_TARGET}-objs4 DEPENDS
                      ${objects})

    set(cobjs)
    set(cobjs-tgt)
    if (NOT "${cc4_C_SOURCES}" STREQUAL "")
        set(cobjs "${cc4_TARGET}-objs")
        set(cobjs-tgt "$<TARGET_OBJECTS:${cobjs}>")
        add_library(${cobjs} OBJECT
                    ${cc4_C_SOURCES})
        generate_warnings(${cobjs} PRIVATE)
        target_link_libraries(${cobjs} PUBLIC
                              c4rt3 BDWgc::gc)
        target_include_directories(${cobjs} PUBLIC
                                   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
                                   $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
    endif ()

    set(arch_depends c4ar ${objects})
    if (NOT "${cc4_C_SOURCES}" STREQUAL "")
        list(APPEND arch_depends "${cobjs-tgt}")
    endif ()

    set(arch "${CMAKE_CURRENT_BINARY_DIR}/${cc4_TARGET}.c4a")
    add_custom_command(OUTPUT "${arch}"
                       DEPENDS ${arch_depends}
                       VERBATIM COMMAND
                       c4ar -o
                       "${arch}"
                       ${objects}
                       ${cobjs-tgt}
                       COMMAND_EXPAND_LISTS
                       COMMENT "Creating C4 library ${cc4_TARGET}.c4a")
    add_custom_target(lib${cc4_TARGET}.c4a ALL DEPENDS "${arch}")

    add_library(${cc4_TARGET} INTERFACE)
    add_dependencies(${cc4_TARGET} lib${cc4_TARGET}.c4a)

    target_include_directories(${cc4_TARGET} INTERFACE
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
                               $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
    target_link_libraries(${cc4_TARGET} INTERFACE
                          $<BUILD_INTERFACE:${arch}>
                          $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_LIBDIR}/${cc4_TARGET}.c4a>)
endfunction()
