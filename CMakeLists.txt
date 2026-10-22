cmake_minimum_required(VERSION 3.23)

project(GeneticTSP LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Gera build/compile_commands.json para o clangd achar os headers em include/.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Tudo exceto o main, para os testes poderem ligar na mesma biblioteca.
add_library(tsp_core
    src/cli.cpp
    src/config.cpp
    src/distance.cpp
    src/points.cpp
)
target_include_directories(tsp_core PUBLIC include)
target_compile_options(tsp_core PUBLIC -Wall -Wextra)

add_executable(ga src/main.cpp)
target_link_libraries(ga PRIVATE tsp_core)
