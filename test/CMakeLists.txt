project(LexerTests CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/Catch2/contrib")

list(APPEND test_files lexer_tests.cpp test_formal_grammar.cpp test_lexer.cpp test_linked_list.cpp test_string_utils.cpp)

add_executable(lexer_tests ${test_files})
target_include_directories(lexer_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(lexer_tests lexer_lib Catch2::Catch2)

include(CTest)
include(Catch)
catch_discover_tests(lexer_tests)
