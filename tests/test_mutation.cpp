#include "mutation.hpp"
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

void testSwapMutation() {
    std::cout << "[Test] swapMutation..." << std::endl;
    Rng rng(42);
    const size_t n = 10;
    Route original(n);
    std::iota(original.begin(), original.end(), 0);

    for (int i = 0; i < 50; ++i) {
        Route r = original;
        swapMutation(r, rng);
        assert(isValidPermutation(r, n));
        // Em um swap de 2 elementos, exatamente 2 posições devem diferir (ou ser permutação válida)
        int diffCount = 0;
        for (size_t j = 0; j < n; ++j) {
            if (r[j] != original[j]) diffCount++;
        }
        assert(diffCount == 2);
    }
    std::cout << "  -> PASS\n";
}

void testInsertionMutation() {
    std::cout << "[Test] insertionMutation..." << std::endl;
    Rng rng(42);
    const size_t n = 10;
    Route original(n);
    std::iota(original.begin(), original.end(), 0);

    for (int i = 0; i < 50; ++i) {
        Route r = original;
        insertionMutation(r, rng);
        assert(isValidPermutation(r, n));
    }
    std::cout << "  -> PASS\n";
}

void testScrambleMutation() {
    std::cout << "[Test] scrambleMutation..." << std::endl;
    Rng rng(42);
    const size_t n = 10;
    Route original(n);
    std::iota(original.begin(), original.end(), 0);

    for (int i = 0; i < 50; ++i) {
        Route r = original;
        scrambleMutation(r, rng);
        assert(isValidPermutation(r, n));
    }
    std::cout << "  -> PASS\n";
}

void testInversionMutation() {
    std::cout << "[Test] inversionMutation..." << std::endl;
    Rng rng(42);
    const size_t n = 10;
    Route original(n);
    std::iota(original.begin(), original.end(), 0);

    for (int i = 0; i < 50; ++i) {
        Route r = original;
        inversionMutation(r, rng);
        assert(isValidPermutation(r, n));
    }
    std::cout << "  -> PASS\n";
}

void testEdgeCases() {
    std::cout << "[Test] mutation edge cases (size 0, 1)..." << std::endl;
    Rng rng(42);

    Route empty;
    swapMutation(empty, rng);
    assert(empty.empty());

    Route single = {0};
    swapMutation(single, rng);
    assert(single.size() == 1 && single[0] == 0);
    insertionMutation(single, rng);
    assert(single.size() == 1 && single[0] == 0);
    scrambleMutation(single, rng);
    assert(single.size() == 1 && single[0] == 0);
    inversionMutation(single, rng);
    assert(single.size() == 1 && single[0] == 0);

    std::cout << "  -> PASS\n";
}

int main() {
    std::cout << "=== Executando Testes de Mutação ===" << std::endl;
    testSwapMutation();
    testInsertionMutation();
    testScrambleMutation();
    testInversionMutation();
    testEdgeCases();
    std::cout << "Todos os testes de mutação passaram com sucesso!\n" << std::endl;
    return 0;
}
