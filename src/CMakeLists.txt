
list(APPEND source_files formal_grammar.c hash_table.c linked_list.c log.c parser.c string_utils.c)

add_library(parser_lib ${source_files})

add_executable(parser main.c)
target_link_libraries(parser parser_lib)
