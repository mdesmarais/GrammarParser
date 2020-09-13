cmake_minimum_required(VERSION 3.0)
project(lexer C)

set(CMAKE_C_STANDARD 99)

option(BUILD_DOC "Build documentation with doxygen" OFF)

add_subdirectory(Catch2)

add_subdirectory(src)
add_subdirectory(test)