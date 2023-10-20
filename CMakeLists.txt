cmake_minimum_required (VERSION 3.5)

project( tic )

add_executable( ${PROJECT_NAME} tic.cpp )

set_target_properties(${PROJECT_NAME} PROPERTIES
            CXX_STANDARD 17
            CXX_EXTENSIONS OFF )
