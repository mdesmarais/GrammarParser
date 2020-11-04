cmake_minimum_required(VERSION 3.0)
project(lexer C)

set(CMAKE_C_STANDARD 99)

option(BUILD_DOC "Build documentation with doxygen" OFF)

add_subdirectory(Catch2)

add_subdirectory(src)

if (BUILD_TESTING)
    add_subdirectory(test)
    install(DIRECTORY test/data DESTINATION ${CMAKE_INSTALL_PREFIX})
    install(TARGETS parser_tests RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})
ENDIF()

install(TARGETS parser RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})
install(DIRECTORY examples DESTINATION ${CMAKE_INSTALL_PREFIX})
