cmake_minimum_required(VERSION 3.15)

project(SAT_solver CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra -Wpedantic)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(src)

enable_testing()

add_test(NAME heuristic_naive COMMAND solver --print-heuristic)
set_tests_properties(heuristic_naive PROPERTIES PASS_REGULAR_EXPRESSION "^naive")
add_test(NAME heuristic_jwh COMMAND solver -jwh --print-heuristic)
set_tests_properties(heuristic_jwh PROPERTIES PASS_REGULAR_EXPRESSION "^jwh")

file(GLOB TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cnf")

foreach(TEST_FILE IN LISTS TEST_FILES)
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
    add_test(NAME ${TEST_NAME} COMMAND solver ${TEST_FILE})
    add_test(NAME ${TEST_NAME}_jwh COMMAND solver ${TEST_FILE} -jwh)

    if(TEST_NAME MATCHES "^uf_")
        set_tests_properties(${TEST_NAME} PROPERTIES PASS_REGULAR_EXPRESSION "SAT")
        set_tests_properties(${TEST_NAME}_jwh PROPERTIES PASS_REGULAR_EXPRESSION "SAT")
    elseif(TEST_NAME MATCHES "^uuf_")
        set_tests_properties(${TEST_NAME} PROPERTIES PASS_REGULAR_EXPRESSION "UNSAT")
        set_tests_properties(${TEST_NAME}_jwh PROPERTIES PASS_REGULAR_EXPRESSION "UNSAT")
    endif()
endforeach()
