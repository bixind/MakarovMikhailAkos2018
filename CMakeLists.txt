cmake_minimum_required(VERSION 3.8)
project(akos2018)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

add_executable(square_solver square_solver/main.cpp)
add_executable(square_solver_tests square_solver/tests.cpp square_solver/mytest.h)

add_subdirectory(gtest)

add_executable(onegin onegin/main.cpp onegin/main.h)
add_executable(onegin_test onegin/tests.cpp onegin/main.h)
target_link_libraries(onegin_test gtest gtest_main)

add_executable(stack stack/stack.cpp stack/stack.h)
add_executable(stack_test stack/tests.cpp stack/stack.h)
target_link_libraries(stack_test gtest gtest_main)

add_executable(cpu cpu/main.cpp cpu/parser.h stack/stack.h)
add_executable(compiler cpu/compiler.cpp cpu/parser.h stack/stack.h cpu/compiler.h)
add_executable(runner cpu/runner.cpp cpu/parser.h stack/stack.h)
add_executable(cpu_test cpu/tests.cpp cpu/parser.h stack/stack.h)
target_link_libraries(cpu_test gtest gtest_main)

add_executable(list_test list/tests.cpp list/list.h)
target_link_libraries(list_test gtest gtest_main)