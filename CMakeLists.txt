cmake_minimum_required(VERSION 3.30)
project(demo)

set(CMAKE_CXX_STANDARD 23)

find_package(mio CONFIG REQUIRED)
find_package(Boost REQUIRED spirit)

add_subdirectory(src/c4)
