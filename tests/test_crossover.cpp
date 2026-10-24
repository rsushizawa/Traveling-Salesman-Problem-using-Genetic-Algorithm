#include "crossover.hpp"
#include <iostream>
#include <cassert>
#include <vector>
#include <numeric>
#include <algorithm>

// Auxiliar para checar se a rota continua sendo uma permutação válida de 0..n-1
bool isValidPermutation(const Route& r, size_t n) {
    if (r.size() != n) return false;
    std::vector<bool> seen(n, false);
    for (int city : r) {
        if (city < 0 || static_cast<size_t>(city) >= n || seen[city]) {
            return false;
        }
        seen[city] = true;
    }
    return true;
}

void testOrderedCrossover() {
    std::cout << "[Test] orderedCrossover (OX)..." << std::endl;
    Rng rng(42);
    const size_t n = 12;

    Route parentA(n);
    std::iota(parentA.begin(), parentA.end(), 0);

    Route parentB = parentA;
    std::reverse(parentB.begin(), parentB.end());

    for (int i = 0; i < 100; ++i) {
        Route child = orderedCrossover(parentA, parentB, rng);
        assert(isValidPermutation(child, n));
    }
    std::cout << "  -> PASS\n";
}

void testOrderBasedCrossover() {
    std::cout << "[Test] orderBasedCrossover (OBX)..." << std::endl;
    Rng rng(42);
    const size_t n = 12;

    Route parentA(n);
    std::iota(parentA.begin(), parentA.end(), 0);

    Route parentB = parentA;
    std::reverse(parentB.begin(), parentB.end());

    for (int i = 0; i < 100; ++i) {
        Route child = orderBasedCrossover(parentA, parentB, rng);
        assert(isValidPermutation(child, n));
    }
    std::cout << "  -> PASS\n";
}

void testUniformCrossoverRepaired() {
    std::cout << "[Test] uniformCrossoverRepaired..." << std::endl;
    Rng rng(42);
    const size_t n = 12;

    Route parentA(n);
    std::iota(parentA.begin(), parentA.end(), 0);

    Route parentB = parentA;
    std::reverse(parentB.begin(), parentB.end());

    for (int i = 0; i < 100; ++i) {
        Route child = uniformCrossoverRepaired(parentA, parentB, rng);
        assert(isValidPermutation(child, n));
    }
    std::cout << "  -> PASS\n";
}

void testEdgeCases() {
    std::cout << "[Test] crossover edge cases (size 0, 1)..." << std::endl;
    Rng rng(42);

    Route empty;
    assert(orderedCrossover(empty, empty, rng).empty());
    assert(orderBasedCrossover(empty, empty, rng).empty());
    assert(uniformCrossoverRepaired(empty, empty, rng).empty());

    Route single = {0};
    assert(orderedCrossover(single, single, rng) == single);
    assert(orderBasedCrossover(single, single, rng) == single);
    assert(uniformCrossoverRepaired(single, single, rng) == single);

    std::cout << "  -> PASS\n";
}

int main() {
    std::cout << "=== Executando Testes de Crossover ===" << std::endl;
    testOrderedCrossover();
    testOrderBasedCrossover();
    testUniformCrossoverRepaired();
    testEdgeCases();
    std::cout << "Todos os testes de crossover passaram com sucesso!\n" << std::endl;
    return 0;
}
