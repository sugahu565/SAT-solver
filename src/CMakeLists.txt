add_executable(solver SATSolver.cpp main.cpp)
target_include_directories(solver PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

