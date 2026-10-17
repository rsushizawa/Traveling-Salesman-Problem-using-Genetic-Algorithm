cmake_minimum_required(VERSION 3.23)

project(GeneticTSP LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(ga src/main.cpp)
target_include_directories(ga PRIVATE include)
target_compile_options(ga PRIVATE -Wall -Wextra)
