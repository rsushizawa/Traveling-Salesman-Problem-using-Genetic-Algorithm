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
    src/crossover.cpp
    src/distance.cpp
    src/mutation.cpp
    src/selection.cpp
)
target_include_directories(tsp_core PUBLIC include)
target_compile_options(tsp_core PUBLIC -Wall -Wextra)

add_executable(ga src/main.cpp)
target_link_libraries(ga PRIVATE tsp_core)

# Executáveis de testes
enable_testing()

add_executable(test_selection tests/test_selection.cpp)
target_link_libraries(test_selection PRIVATE tsp_core)
add_test(NAME SelectionTest COMMAND test_selection)

add_executable(test_mutation tests/test_mutation.cpp)
target_link_libraries(test_mutation PRIVATE tsp_core)
add_test(NAME MutationTest COMMAND test_mutation)

add_executable(test_crossover tests/test_crossover.cpp)
target_link_libraries(test_crossover PRIVATE tsp_core)
add_test(NAME CrossoverTest COMMAND test_crossover)
