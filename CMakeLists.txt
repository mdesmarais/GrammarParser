cmake_minimum_required(VERSION 3.17)
project(lexer C)

set(CMAKE_C_STANDARD 99)

add_subdirectory(Catch2)

add_subdirectory(src)
add_subdirectory(test)