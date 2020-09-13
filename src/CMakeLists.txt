
find_package(Doxygen REQUIRED)

if (DOXYGEN_FOUND)
    set(DOXYGEN_IN ${CMAKE_SOURCE_DIR}/docs/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_BINARY_DIR}/Doxyfile)

    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

    add_custom_target(doc_doxygen ALL COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT} WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
else()
    message(FATAL_ERROR "Doxygen is needed to generate the documentation")
endif()

list(APPEND source_files
        collections/hash_table.c
        collections/linked_list.c
        collections/set.c
        formal_grammar.c
        hash.c
        log.c
        parser.c
        parser_errors.c
        range.c
        string_utils.c
)

add_library(parser_lib ${source_files})

add_executable(parser main.c)
target_link_libraries(parser parser_lib)
