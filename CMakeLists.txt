cmake_minimum_required(VERSION 3.15)

project(SAT_solver CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra -Wpedantic)

add_subdirectory(src)

enable_testing()

file(GLOB TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cnf")

foreach(TEST_FILE IN LISTS TEST_FILES)
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
    add_test(NAME ${TEST_NAME} COMMAND solver ${TEST_FILE})

    if(TEST_NAME MATCHES "^uf_")
        set_tests_properties(${TEST_NAME} PROPERTIES PASS_REGULAR_EXPRESSION "SAT")
    elseif(TEST_NAME MATCHES "^uuf_")
        set_tests_properties(${TEST_NAME} PROPERTIES PASS_REGULAR_EXPRESSION "UNSAT")
    endif()
endforeach()
