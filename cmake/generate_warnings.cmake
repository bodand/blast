# cg3 project
#
# Copyright (c) 2022, 2026, András Bodor <bodand@proton.me>
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
# cmake/generate_warnings.cmake --
#   Generates a set of warnings available on the currently used compiler.
#   Sets them on the given interface target; this is to be used while compiling
#   every other target to all have the full set of available warnings.

include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)
include(CheckLinkerFlag)

function(checkwarningflag OptionName CacheName)
    if (OptionName MATCHES [[^/|^-]])
        set(FlagPrefix "")
    else ()
        set(FlagPrefix "-W")
    endif ()
    set(flag ${FlagPrefix}${OptionName})

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

    check_cxx_compiler_flag("${flag}" "HasCFlag_${CacheName}")
    check_c_compiler_flag("${flag}" "HasCXXFlag_${CacheName}")

    unset(CMAKE_REQUIRED_FLAGS)

    set("HAS_CXX_FLAG_${CacheName}" ${HasCFlag_${CacheName}} PARENT_SCOPE)
    set("HAS_C_FLAG_${CacheName}" ${HasCXXFlag_${CacheName}} PARENT_SCOPE)
endfunction()

function(generate_warnings _Interface Mode)
    set(gw_known_warnings
        -ffunction-sections -fdata-sections
        -fstack-protector-strong
        -D_FORTIFY_SOURCE=3
        /permissive-
        /Zc:__cplusplus /Zc:preprocessor /EHsc
        # GCC/Clang
        extra pedantic sign-compare error=uninitialized unused cast-qual cast-align=strict
        abstract-vbase-init array-bounds-pointer-arithmetic assign-enum consumed
        conditional-uninitialized deprecated-implementations header-hygiene error=move
        error=documentation-deprecated-sync error=non-virtual-dtor error=infinite-recursion
        keyword-macro loop-analysis newline-eof over-aligned redundant-parens
        reserved-id-macro sign-conversion signed-enum-bitfield thread-safety
        undefined-internal-type undefined-reinterpret-cast unused-const-variable
        unneeded-internal-declaration unreachable-code-aggressive unused-variable
        unused-exception-parameter unused-parameter unused-template error=lifetime
        error=sometimes-uninitialized tautological-overlap-compare suggest-final-types
        nullability-completeness unreachable-code-loop-increment redundant-decls
        suggest-attribute=pure suggest-attribute=const suggest-attribute=cold
        suggest-final-methods duplicated-branches placement-new=2 error=trampolines
        covered-switch-default error=vla error=implicit-fallthrough format=2
        logical-op reorder lifetime
        no-shadow
        no-changes-meaning # stfu
        no-unsafe-buffer-usage # todo
        no-exit-time-destructors # todo
        no-unknown-pragmas # really don't gaf
        no-switch-default # clashes with covered-switch-default
        # ## random compat stuffs i don't care about ##
        no-c++98-compat no-c++98-compat-pedantic no-c++20-compat
        no-c2y-extensions # catches Catch2's __COUNTER__ usage
        # MSVC
        /w14062 /w14165 /w14191 /w14242 /we4263 /w14265 /w14287 /w14296 /we4350 /we4355
        /w14355 /w14471 /we4545 /w14546 /w14547 /w14548 /w14549 /w14557 /we4596 /w14605
        /w14668 /w14768 /w14822 /we4837 /we4928 /we4946 /we4986 /w15032 /w15039 /wd4010
        /wd5030 /w14061 /sdl /RTC1
        4
        /diagnostics:caret
        )
    set(gw_known_linker_flags
        "-Wl,--gc-sections" # GCC/Clang strip dead sections
        "-Wl,--no-undefined"
        "-Wl,-undefined,error"
        "-Wl,-z,relro"
        "-Wl,-z,now"
        "/OPT:REF"          # MSVC strip dead sections
        "/OPT:ICF"          # MSVC fold identical COMDATs
        )

    # cannot just check if -Wall, because MSVC also has /Wall, but also accepts -Wall
    # this wouldn't be a problem, but /Wall means literally everything... don't.
    set(gw_found_warnings
        "$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CXX_COMPILER_ID:GNU,Clang,AppleClang>>:-Wall>"
        "$<$<AND:$<COMPILE_LANGUAGE:C>,$<C_COMPILER_ID:GNU,Clang,AppleClang>>:-Wall>"
        "$<$<AND:$<CONFIG:Debug>,$<OR:$<CXX_COMPILER_ID:GNU,Clang,AppleClang>,$<C_COMPILER_ID:GNU,Clang,AppleClang>>>:-Og>"
        )
    set(gw_found_linker_flags "")

    ############################################################################
    # compiler
    ############################################################################
    foreach (warn IN LISTS gw_known_warnings)
        string(MAKE_C_IDENTIFIER "${warn}" CacheName)
        checkwarningflag("${warn}" ${CacheName})

        if (warn MATCHES "^[-/]")
            set(WarningPrefix "")
        else ()
            set(WarningPrefix "-W")
        endif ()
        set(flag "${WarningPrefix}${warn}")

        if (HAS_CXX_FLAG_${CacheName})
            list(APPEND gw_found_warnings "$<$<COMPILE_LANGUAGE:CXX>:${flag}>")
        endif ()

        if (HAS_C_FLAG_${CacheName})
            list(APPEND gw_found_warnings "$<$<COMPILE_LANGUAGE:C>:${flag}>")
        endif ()
    endforeach ()
    target_compile_options("${_Interface}" ${Mode} ${gw_found_warnings})

    ############################################################################
    # linker
    ############################################################################
    foreach (lflag IN LISTS gw_known_linker_flags)
        string(MAKE_C_IDENTIFIER "${lflag}" CacheName)

        check_linker_flag(CXX "${lflag}" "HasCXXLinker_${CacheName}")
        if (HasCXXLinker_${CacheName})
            list(APPEND gw_found_linker_flags "$<$<LINK_LANGUAGE:CXX>:${lflag}>")
        endif ()

        check_linker_flag(C "${lflag}" "HasCLinker_${CacheName}")
        if (HasCLinker_${CacheName})
            list(APPEND gw_found_linker_flags "$<$<LINK_LANGUAGE:C>:${lflag}>")
        endif ()
    endforeach ()

    target_link_options("${_Interface}" ${Mode} ${gw_found_linker_flags})
endfunction()
