cmake_minimum_required(VERSION 3.30)
project(demo)

set(CMAKE_CXX_STANDARD 23)

find_package(mio CONFIG REQUIRED)
find_package(Boost REQUIRED spirit)
find_package(fmt CONFIG REQUIRED)
find_package(pcre2 CONFIG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(uni-algo CONFIG REQUIRED)

include(FetchContent)
FetchContent_Declare(
        libassert
        GIT_REPOSITORY https://github.com/jeremy-rifkin/libassert.git
        GIT_TAG        v2.1.5 # <HASH or TAG>
        )
FetchContent_MakeAvailable(libassert)

add_subdirectory(src/c4)
