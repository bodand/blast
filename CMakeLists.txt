cmake_minimum_required(VERSION 3.30)
project(demo)

set(CMAKE_CXX_STANDARD 23)

find_package(mio CONFIG REQUIRED)
find_package(Boost REQUIRED spirit)

add_executable(demo
               src/parser/ast.hxx
               src/parser/ast_adapted.hxx
               src/parser/parser.hxx
               src/parser/parser_def.hxx
               src/parser/parser.cxx
               src/main.cxx)

target_link_libraries(demo PRIVATE
                      Boost::spirit
                      mio::mio_min_winapi)

add_subdirectory(src/c4)
