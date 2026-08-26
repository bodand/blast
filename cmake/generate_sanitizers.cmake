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
# cmake/generate_sanitizers.cmake --
#   Generate flags to enable sanitizers

include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)
include(CheckLinkerFlag)

function(generate_sanitizers _Target)
    if ("${C4_SANITIZERS}" STREQUAL "")
        return()
    endif ()

    if (CMAKE_C_COMPILER_ID MATCHES "MSVC" OR CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        set(STRICT_FLAG "/WX")
    elseif (CMAKE_C_COMPILER_ID MATCHES "GCC|Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GCC|Clang")
        set(STRICT_FLAG "-Werror")
    else ()
        # XXX pray the cc works as we think it does and properly chokes on
        # invalid flags now and not in the future
        set(STRICT_FLAG "")
    endif ()

    set(CMAKE_REQUIRED_FLAGS "${STRICT_FLAG}")

    check_cxx_compiler_flag("-fsanitize=${C4_SANITIZERS}" "HasCFlag_FSanitize")
    check_c_compiler_flag("-fno-omit-frame-pointer" "HasCXXFlag_FNoOmitFramePointer")
    check_linker_flag(C "-fsanitize=${C4_SANITIZERS}" "HasCLinkFlag_FSanitize")
    check_linker_flag(CXX "-fsanitize=${C4_SANITIZERS}" "HasCLinkFlag_FSanitize")

    unset(CMAKE_REQUIRED_FLAGS)

    target_compile_options(${_Target} PRIVATE
                           "$<$<AND:$<BOOL:${HasCFlag_FSanitize}>,$<COMPILE_LANGUAGE:C>>:-fsanitize=${C4_SANITIZERS}>"
                           "$<$<AND:$<BOOL:${HasCXXFlag_FSanitize}>,$<COMPILE_LANGUAGE:CXX>>:-fsanitize=${C4_SANITIZERS}>"
                           "$<$<AND:$<BOOL:${HasCFlag_FNoOmitFramePointer}>,$<COMPILE_LANGUAGE:C>>:-fno-omit-frame-pointer>"
                           "$<$<AND:$<BOOL:${HasCXXFlag_FNoOmitFramePointer}>,$<COMPILE_LANGUAGE:CXX>>:-fno-omit-frame-pointer>"
                           )
    target_link_options(${_Target} PRIVATE
                        "$<$<AND:$<BOOL:${HasCLinkFlag_FSanitize}>,$<LINK_LANGUAGE:C>>:-fsanitize=${C4_SANITIZERS}>"
                        "$<$<AND:$<BOOL:${HasCXXLinkFlag_FSanitize}>,$<LINK_LANGUAGE:CXX>>:-fsanitize=${C4_SANITIZERS}>"
                        )
endfunction()
