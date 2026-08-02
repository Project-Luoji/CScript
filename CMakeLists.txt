cmake_minimum_required(VERSION 3.15)

project(Dyno
        VERSION 1.0.0
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(Cscript
        test.cpp
)

target_include_directories(Cscript PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)