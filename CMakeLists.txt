cmake_minimum_required(VERSION 3.16)

project(tictactoe LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Wpedantic)

add_library(tictactoe_lib tic.cpp ai.cpp)
target_include_directories(tictactoe_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(tic main.cpp)
target_link_libraries(tic PRIVATE tictactoe_lib)

find_package(Qt6 QUIET COMPONENTS Widgets)
if (Qt6_FOUND)
    message(STATUS "Qt6 found. Building GUI target: tic_gui")
    add_executable(tic_gui gui.cpp)
    target_link_libraries(tic_gui PRIVATE Qt6::Widgets tictactoe_lib)
else()
    message(STATUS "Qt6 not found. GUI target is disabled. CLI build remains available.")
endif()

enable_testing()
add_executable(tictactoe_tests tests.cpp)
target_link_libraries(tictactoe_tests PRIVATE tictactoe_lib)
add_test(NAME tictactoe_tests COMMAND tictactoe_tests)
