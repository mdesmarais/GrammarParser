project(LexerTests CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/Catch2/contrib")

list(APPEND test_files helpers.cpp parser_tests.cpp test_formal_grammar.cpp test_hash_table.cpp test_linked_list.cpp test_parser.cpp test_string_utils.cpp)

add_executable(parser_tests ${test_files})
target_include_directories(parser_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(parser_tests parser_lib Catch2::Catch2)

include(CTest)
include(Catch)
catch_discover_tests(parser_tests WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
