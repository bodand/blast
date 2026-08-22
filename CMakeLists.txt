cmake_minimum_required(VERSION 3.30)

if (CMAKE_VERSION VERSION_LESS 4.0)
    cmake_policy(SET CMP0065 NEW) # disable automatic dynamic exports for exes
endif ()

cmake_policy(SET CMP0048 NEW) # project() sets PROJECT_VERSION
cmake_policy(SET CMP0077 NEW) # option() doesn't break local variables
cmake_policy(SET CMP0092 NEW) # don't special-case MSVC warning flags
cmake_policy(SET CMP0096 NEW) # don't drop leading zero from versions
cmake_policy(SET CMP0110 NEW) # add_test support names with whitespace
cmake_policy(SET CMP0111 NEW) # imported targets require a location
cmake_policy(SET CMP0115 NEW) # require explicit extensions for source file
cmake_policy(SET CMP0118 NEW) # GENERATED property is globally readable
cmake_policy(SET CMP0125 NEW) # find_* read local variables
cmake_policy(SET CMP0127 NEW) # cmake_dependent_option proper syntax rules
cmake_policy(SET CMP0135 NEW) # Fix downloaded & extracted files' timestamps
cmake_policy(SET CMP0147 NEW) # Enable parallel MsBuild of custom targets
cmake_policy(SET CMP0152 NEW) # resolve symlink before collapsing paths
cmake_policy(SET CMP0156 NEW) # if linker allows it, be smarter about linking
cmake_policy(SET CMP0167 NEW) # use new Boost finding cmake code

if (CMAKE_VERSION VERSION_GREATER_EQUAL 3.31)
    cmake_policy(SET CMP0175 NEW) # more rigorous add_custom_command arg-parsing
    cmake_policy(SET CMP0176 NEW) # use UTF-8 in execute process
    cmake_policy(SET CMP0179 NEW) # when de-duping linked libs, keep first
endif ()
if (CMAKE_VERSION VERSION_GREATER_EQUAL 4.0)
    cmake_policy(SET CMP0184 NEW) # MSVC dynamic analysis flags handling
endif ()

project(blAST
        VERSION 0.1.2
        DESCRIPTION "Scriptable AST scanner"
        LANGUAGES C CXX)

# maybe some time...
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

option(BLAST_BUILD_TESTS "Build the test suite for blAST [Yes]" Yes)
option(BLAST_FORBID_LTO "Instruct the build-system to never enable LTO [No]" No)
option(BLAST_ENABLE_SANITIZERS "Build blAST with sanitizers [No]" No)
# Currently broken, needs more wizardry
option(BLAST_USE_MIMALLOC "Use mimalloc memory allocator instead of malloc in libc [No]" No)

set(CMAKE_CXX_STANDARD 23)

include(GNUInstallDirs)
include(CheckCXXSymbolExists)
include(CMakeDependentOption)
include(InstallRequiredSystemLibraries)
include(FetchContent)
include(TestBigEndian)
include(CTest)
include(FetchContent)

include(cmake/add_htmldoc.cmake)
include(cmake/add_manpage.cmake)
include(cmake/add_covered_test.cmake)
include(cmake/generate_warnings.cmake)
include(cmake/generate_lto.cmake)
include(cmake/get_arch.cmake)

get_arch(BLAST_ARCH)

# coverage data
add_library(internal-coverage INTERFACE)
if (NOT DEFINED ENV{CG_DEPLOYMENT})
    target_compile_options(internal-coverage INTERFACE
                           $<$<CXX_COMPILER_ID:Clang>:-fprofile-instr-generate>
                           $<$<CXX_COMPILER_ID:Clang>:-fcoverage-mapping>)
    if ("x${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}" STREQUAL "xMSVC"
        AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        cmake_path(GET CMAKE_CXX_COMPILER PARENT_PATH vs_llvmx64_bindir)
        cmake_path(GET vs_llvmx64_bindir PARENT_PATH vs_llvmx64_dir)
        if (CMAKE_CXX_COMPILER_VERSION MATCHES [[^([1-9][0-9]*)\.]])
            set(clang_profile_lib "${vs_llvmx64_dir}/lib/clang/${CMAKE_MATCH_1}/lib/windows/clang_rt.profile-${BLAST_ARCH}.lib")
            target_link_libraries(internal-coverage INTERFACE "${clang_profile_lib}")
        else ()
            message(WARNING
                    "Cannot deduce Clang Runtime location: cannot calculate the major version of the compiler is from: ${CMAKE_CXX_COMPILER_VERSION}")
        endif ()
    else ()
        target_link_options(internal-coverage INTERFACE
                            $<$<CXX_COMPILER_ID:Clang>:-fprofile-instr-generate>
                            $<$<CXX_COMPILER_ID:Clang>:-fcoverage-mapping>)
    endif ()
endif ()

# warnings
add_library(internal-warnings INTERFACE)
generate_warnings(internal-warnings)

# LTO
add_library(internal-lto INTERFACE)
generate_lto(internal-lto)

add_compile_options(-fstack-protector-strong)

# ASan
if (BLAST_ENABLE_SANITIZERS)
    if (CMAKE_CXX_COMPILER_ID MATCHES [[GNU|Clang]])
        add_compile_options(
                $<$<PLATFORM_ID:Linux>:-fsanitize=address,undefined>
                $<$<PLATFORM_ID:Linux>:-fno-omit-frame-pointer>
                )
        add_link_options(
                $<$<PLATFORM_ID:Linux>:-fsanitize=address,undefined>
                )
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        add_compile_options(
                /fsanitize=address
                /fsanitize-address-use-after-return
                )
        #        add_link_options(
        #                clang_rt.asan_$<$<CONFIG:DEBUG>:dbg_>dynamic-${BLAST_ARCH}.lib
        #                clang_rt.asan_dynamic_runtime_thunk-${BLAST_ARCH}.lib
        #                )
    endif ()
endif ()

# MSVC argument injection
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    message(STATUS "Using MSVC: ${CMAKE_CXX_COMPILER}")
    message(STATUS "Injecting /permissive- into cl command-line")
    add_compile_options(/permissive-)
    message(STATUS "Injecting /Zc:__cplusplus into cl command-line")
    add_compile_options(/Zc:__cplusplus)
    message(STATUS "Injecting /Zc:preprocessor into cl command-line")
    add_compile_options(/Zc:preprocessor)
endif ()
if ("x${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}" STREQUAL "xMSVC")
    message(STATUS "Using clang-cl: ${CMAKE_CXX_COMPILER}")
    message(STATUS "Injecting /EHsc into clang-cl command-line")
    add_compile_options(/EHsc)
endif ()

# dependencies
find_package(mio CONFIG REQUIRED)
find_package(pcre2 CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(libassert CONFIG REQUIRED)
find_package(uni-algo CONFIG REQUIRED)
find_package(lyra CONFIG REQUIRED)
find_package(ryu CONFIG REQUIRED)
find_package(BDWgc CONFIG REQUIRED)
add_compile_definitions(GC_THREADS)

find_package(zstd REQUIRED)
if (NOT TARGET zstd::libzstd_shared)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(ZSTD REQUIRED IMPORTED_TARGET libzstd)
    add_library(zstd::libzstd_shared ALIAS PkgConfig::ZSTD)
endif ()

## Mio wrap Win specific stuffs
add_library(c4-mio INTERFACE)
target_link_libraries(c4-mio INTERFACE
                      $<$<PLATFORM_ID:Windows>:mio::mio_min_winapi>
                      $<$<NOT:$<PLATFORM_ID:Windows>>:mio::mio>
                      )

## LLVM ##
find_package(LLVM CONFIG REQUIRED PATHS /usr/lib/llvm/22/lib/cmake/llvm NO_DEFAULT_PATH)
#find_library(LLVM_OBJECT_LIB NAMES LLVMObject PATHS /usr/lib/llvm/22/lib)
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")

## Clang ##
find_package(Clang CONFIG REQUIRED PATHS /usr/lib/llvm/22/lib/cmake/clang NO_DEFAULT_PATH)

add_library(c4_llvm INTERFACE)
target_compile_definitions(c4_llvm INTERFACE ${LLVM_DEFINITIONS_LIST})
target_include_directories(c4_llvm SYSTEM INTERFACE
                           ${LLVM_INCLUDE_DIRS}
                           ${CLANG_INCLUDE_DIRS})
target_link_libraries(c4_llvm INTERFACE clang-cpp LLVM
#                      ${LLVM_OBJECT_LIB}
                      )

#add_subdirectory(src/cdb)
add_subdirectory(src/sgetopt)

add_subdirectory(src/c4rt2)
add_subdirectory(src/libarray)
add_subdirectory(src/std4)
add_subdirectory(src/blast)

find_package(raylib)
if (TARGET raylib)
    add_subdirectory(src/raylib)
endif ()

add_subdirectory(src/c4)
add_subdirectory(src/c4c)
add_subdirectory(src/c4ar)
#add_subdirectory(src/c4i)

add_subdirectory(test)
