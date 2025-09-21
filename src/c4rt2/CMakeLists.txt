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
# Originally created: 2025-08-21.
#
# src/c4rt2/CMakeLists.txt --
#   c4rt2 provides two interfaces: c4rt2 and c4rt2c. Both of them provide
#   aim at providing the runtime functionality required by C4, but c4rt2
#   provides this using a stable ABI interface in C. This can be linked against
#   using LLVM's JIT runtime or a normal linker when using c4c while c4rt2c
#   provides a subset of this functionality in an unstable C++ API meant to only
#   be used by c4c and c4 when generating LLVM IR.
#   Most code is ported over from c4rt.

include(CheckTypeSize)

set(clang-tools-location-hint)
set(INC_MIMALLOC)
if (IS_DIRECTORY "${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}")
    set(clang-tools-location-hint
        "HINTS"
        "${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/tools/llvm/")
    set(INC_MIMALLOC
        "-I${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/include")
endif ()

find_program(CLANG_EXEC clang
             NAMES clang clang-cl
             ${clang-tools-location-hint}
             DOC "Clang: used to generate LLVM bitcode from C code"
             REQUIRED)
find_program(CLANGXX_EXEC clang++
             NAMES clang++ clang-cl
             ${clang-tools-location-hint}
             DOC "Clang++: used to generate LLVM bitcode from C++ code"
             REQUIRED)
find_program(OPT_EXEC opt
             ${clang-tools-location-hint}
             DOC "LLVM optimizer for optimizing bitcode in release mode"
             REQUIRED)
find_program(LLVM_LINK_EXEC llvm-link
             ${clang-tools-location-hint}
             DOC "Tool to use for merging LLVM bitcode files"
             REQUIRED)

set(opt $<CONFIG:Release,RelWithDebInfo,MinSizeRel>)
function(compile_into_bitcode in CC)
    set(out "${in}.bc")
    set(optim "${out}.opt")
    add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${out}"
                       DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${in}"
                       "${CMAKE_CURRENT_SOURCE_DIR}/src/c4rt2/dynamic_call_hacks.h"
                       "${CMAKE_CURRENT_SOURCE_DIR}/include/c4rt2/c4rt_package_versions.h"
                       "${CMAKE_CURRENT_SOURCE_DIR}/src/c4rt2/package.1.h"
                       COMMAND "${CC}" -c -emit-llvm
                       -fvisibility=hidden -g
                       -DC4RT2_STATIC
                       -D_CRT_NONSTDC_NO_DEPRECATE
                       -D_CRT_SECURE_NO_WARNINGS
                       ${ARGN}
                       $<${opt}:-DNDEBUG>
                       -I "${CMAKE_CURRENT_BINARY_DIR}"
                       -I "${CMAKE_CURRENT_SOURCE_DIR}/include"
                       -o "${CMAKE_CURRENT_BINARY_DIR}/${out}"
                       "${CMAKE_CURRENT_SOURCE_DIR}/${in}"
                       VERBATIM)
    add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${optim}"
                       DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${out}"
                       COMMAND "$<IF:${opt},${OPT_EXEC},cmake>"
                       $<IF:${opt},--tailcallopt,-E>
                       $<IF:${opt},--hot-cold-split,cat>
                       "${CMAKE_CURRENT_BINARY_DIR}/${out}"
                       "$<IF:${opt},-p,${CMAKE_CURRENT_SOURCE_DIR}/src/empty.txt>"
                       "$<IF:${opt},default<O3$<ANGLE-R>,${CMAKE_CURRENT_SOURCE_DIR}/src/empty.txt>"
                       $<IF:${opt},-o,$<ANGLE-R>> "${CMAKE_CURRENT_BINARY_DIR}/${optim}"
                       VERBATIM)
endfunction()

function(link_bitcode out)
    add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${out}"
                       DEPENDS "$<LIST:TRANSFORM,${ARGN},PREPEND,${CMAKE_CURRENT_BINARY_DIR}/>"
                       COMMAND "${LLVM_LINK_EXEC}" -o "${CMAKE_CURRENT_BINARY_DIR}/${out}"
                       "$<LIST:TRANSFORM,${ARGN},PREPEND,${CMAKE_CURRENT_BINARY_DIR}/>"
                       VERBATIM
                       COMMAND_EXPAND_LISTS)
endfunction()

check_type_size(void* PVOID_SIZE BUILTIN_TYPES_ONLY LANGUAGE C)

set(C4RT2_USE_MIMALLOC NO)
configure_file(src/dll-config.in.h dll-config.h @ONLY)

add_library(c4rt2-core OBJECT
            src/c4rt2/DllMain.c
            src/c4rt2/c4rt_check.cxx
            include/c4rt2/c4rt.h
            src/c4rt2/c4rt.c
            include/c4rt2/package.h
            src/c4rt2/package.c
            include/c4rt2/c4rt_package_versions.h
            src/c4rt2/package.1.h
            src/c4rt2/package.1.c
            include/c4rt2/datum.h
            src/c4rt2/datum.c
            include/c4rt2/datum_type.h
            include/c4rt2/package_type.h
            src/c4rt2/dynamic_call_hacks.h
            )

target_include_directories(c4rt2-core
                           PUBLIC $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
                           PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                           PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)
target_compile_definitions(c4rt2-core PRIVATE $<BUILD_INTERFACE:C4RT2_EXPORTS>)

add_library(c4rt2 SHARED)
target_link_libraries(c4rt2 PUBLIC c4rt2-core BDWgc::gc)

add_library(c4rt2-internal STATIC)
target_link_libraries(c4rt2-internal PUBLIC c4rt2-core)

# Static compile into LLVM bitcode used by both c4c and c4j for generated C4 code
compile_into_bitcode(src/c4rt2/c4rt.c "${CLANG_EXEC}" ${INC_MIMALLOC})
compile_into_bitcode(src/c4rt2/package.1.c "${CLANG_EXEC}" ${INC_MIMALLOC})
compile_into_bitcode(src/c4rt2/package.c "${CLANG_EXEC}" ${INC_MIMALLOC})
compile_into_bitcode(src/c4rt2/datum.c "${CLANG_EXEC}" ${INC_MIMALLOC})

add_library(c4rt2c STATIC
            include/c4rt2c/c4rt_emitter.hxx
            src/c4rt2c/c4rt_emitter.cxx

            include/c4rt2c/ffm_ir_emitter.hxx
            src/c4rt2c/ffm_ir_emitter.cxx

            include/c4rt2c/llvm_value_attribute.hxx
            src/c4rt2c/llvm_value_attribute.cxx

            include/c4rt2c/source_file.hxx
            src/c4rt2c/source_file.cxx
            )
target_link_libraries(c4rt2c
                      PUBLIC
                      c4rt2-internal
                      mio::mio_min_winapi
                      PRIVATE
                      c4
                      c4_llvm
                      libassert::assert)
target_include_directories(c4rt2c
                           PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
