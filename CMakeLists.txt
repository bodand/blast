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

set(default_sanitizers)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(default_sanitizers "address,undefined")
endif ()

option(C4_BUILD_TESTS "Build the test suite for C4 [Yes]" Yes)
option(C4_FORBID_LTO "Instruct the build-system to never enable LTO [No]" No)
set(C4_SANITIZERS ${default_sanitizers} CACHE STRING "Enable sanitizers")

set(CMAKE_CXX_STANDARD 23)

include(GNUInstallDirs)
include(CheckCXXSymbolExists)
include(CMakeDependentOption)
include(InstallRequiredSystemLibraries)
include(FetchContent)
include(TestBigEndian)
include(CTest)
include(FetchContent)

include(cmake/add_c4_library.cmake)
include(cmake/add_covered_test.cmake)
include(cmake/generate_sanitizers.cmake)
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

# dependencies
find_package(mio CONFIG REQUIRED)
find_package(pcre2 CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(libassert CONFIG REQUIRED)
find_package(uni-algo CONFIG REQUIRED)
find_package(BDWgc CONFIG REQUIRED)
add_compile_definitions(GC_THREADS)

find_package(LIEF REQUIRED)

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
target_link_libraries(c4_llvm INTERFACE clang-cpp LLVM)

## Vendored special sub-libraries
add_subdirectory(src/libsgetopt) # sgetopt from skalibs
add_subdirectory(src/libryu)     # d2s* from libryu

## The main language libraries
add_subdirectory(src/libc4)
add_subdirectory(src/libc4rt2)

## Executables
add_subdirectory(src/c4c)
add_subdirectory(src/c4ar)
add_subdirectory(src/c4if)
#add_subdirectory(src/c4i)

## C4 libraries
add_subdirectory(src/libarray.c4)
add_subdirectory(src/libstd4.c4)
add_subdirectory(src/libblast.c4)
add_subdirectory(src/libtap.c4)

find_package(raylib)
if (TARGET raylib)
    add_subdirectory(src/libraylib.c4)
endif ()

add_subdirectory(test)

install(EXPORT c4std
        FILE c4std.cmake
        NAMESPACE c4std::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/c4std)
