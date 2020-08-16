
list(APPEND source_files formal_grammar.c hash_table.c lexer.c linked_list.c log.c set.c string_utils.c)

add_library(lexer_lib ${source_files})

add_executable(lexer main.c)
target_link_libraries(lexer lexer_lib)
