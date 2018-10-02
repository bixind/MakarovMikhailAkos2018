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

add_executable(stack stack/main.cpp stack/main.h)
add_executable(stack_test stack/tests.cpp stack/main.h)
target_link_libraries(stack_test gtest gtest_main)