
list(APPEND targets test_formal_grammar test_lexer test_linked_list test_string_utils)

FOREACH(target ${targets})
    add_executable(${target} ${target}.c)
    target_include_directories(${target} PRIVATE ${PROJECT_SOURCE_DIR}/src)
    target_link_libraries(${target} lexer_lib)
    add_test(NAME ${target} COMMAND ${target})
ENDFOREACH()