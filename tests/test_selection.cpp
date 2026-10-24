#include "selection.hpp"
#include <iostream>
#include <cassert>
#include <map>
#include <numeric>

void testAlphaSelect() {
    std::cout << "[Test] alphaSelect..." << std::endl;
    Rng rng(42);

    Population pop(5);
    pop[0].length = 30.0;
    pop[1].length = 20.0;
    pop[2].length = 10.0; // Melhor
    pop[3].length = 40.0;
    pop[4].length = 50.0;

    int alpha = alphaSelect(pop, rng);
    assert(alpha == 2);

    // Teste com o melhor na primeira posição
    pop[0].length = 5.0;
    alpha = alphaSelect(pop, rng);
    assert(alpha == 0);

    // Teste com o melhor na última posição
    pop[4].length = 1.0;
    alpha = alphaSelect(pop, rng);
    assert(alpha == 4);

    std::cout << "  -> PASS\n";
}

void testTournamentSelect() {
    std::cout << "[Test] tournamentSelect..." << std::endl;
    Rng rng(42);

    Population pop(10);
    for (size_t i = 0; i < pop.size(); ++i) {
        pop[i].length = 100.0 - static_cast<double>(i) * 5.0; // O último (índice 9) tem o menor length (55.0)
    }

    // k = 1: deve retornar um índice válido qualquer
    for (int i = 0; i < 20; ++i) {
        int sel = tournamentSelect(pop, 1, rng);
        assert(sel >= 0 && sel < static_cast<int>(pop.size()));
    }

    // Para k grande, a probabilidade de sortear o melhor índice 9 se aproxima de 1
    // (1 - (9/10)^50 ≈ 0.995). Fazendo 50 sorteios com k=50:
    for (int i = 0; i < 20; ++i) {
        int sel = tournamentSelect(pop, 50, rng);
        assert(sel == 9);
    }

    // k moderado: vencedor deve ser melhor ou igual a qualquer competidor sorteado
    for (int i = 0; i < 50; ++i) {
        int sel = tournamentSelect(pop, 3, rng);
        assert(sel >= 0 && sel < static_cast<int>(pop.size()));
    }

    std::cout << "  -> PASS\n";
}

void testRouletteSelect() {
    std::cout << "[Test] rouletteSelect..." << std::endl;
    Rng rng(42);

    Population pop(4);
    // Indivíduo 0 tem length bem menor (maior fitness: 1/10 = 0.10)
    // Indivíduo 3 tem length muito grande (menor fitness: 1/1000 = 0.001)
    pop[0].length = 10.0;
    pop[1].length = 50.0;
    pop[2].length = 100.0;
    pop[3].length = 1000.0;

    std::map<int, int> counts;
    const int N_TRIALS = 10000;
    for (int i = 0; i < N_TRIALS; ++i) {
        int sel = rouletteSelect(pop, rng);
        assert(sel >= 0 && sel < static_cast<int>(pop.size()));
        counts[sel]++;
    }

    // Verifica se a probabilidade respeita a ordem inversa do length
    assert(counts[0] > counts[1]);
    assert(counts[1] > counts[2]);
    assert(counts[2] > counts[3]);

    std::cout << "  -> PASS\n";
}

void testTruncationSelect() {
    std::cout << "[Test] truncationSelect..." << std::endl;
    Rng rng(42);

    Population pop(10);
    // Definir comprimentos arbitrários
    // 0: 50, 1: 10 (2º), 2: 80, 3: 5 (1º), 4: 90, 5: 15 (3º), 6: 100, 7: 70, 8: 60, 9: 40
    pop[0].length = 50.0;
    pop[1].length = 10.0;
    pop[2].length = 80.0;
    pop[3].length = 5.0;
    pop[4].length = 90.0;
    pop[5].length = 15.0;
    pop[6].length = 100.0;
    pop[7].length = 70.0;
    pop[8].length = 60.0;
    pop[9].length = 40.0;

    // Top 20% de 10 = 2 melhores: índices 3 (len 5) e 1 (len 10)
    for (int i = 0; i < 50; ++i) {
        int sel = truncationSelect(pop, 0.2, rng);
        assert(sel == 3 || sel == 1);
    }

    // Top 30% de 10 = 3 melhores: índices 3 (5), 1 (10), 5 (15)
    for (int i = 0; i < 50; ++i) {
        int sel = truncationSelect(pop, 0.3, rng);
        assert(sel == 3 || sel == 1 || sel == 5);
    }

    // Fraction muito pequena (ex: 0.05) deve selecionar pelo menos 1 indivíduo (o melhor absoluto = 3)
    for (int i = 0; i < 20; ++i) {
        int sel = truncationSelect(pop, 0.05, rng);
        assert(sel == 3);
    }

    std::cout << "  -> PASS\n";
}

int main() {
    std::cout << "=== Executando Testes de Seleção ===" << std::endl;
    testAlphaSelect();
    testTournamentSelect();
    testRouletteSelect();
    testTruncationSelect();
    std::cout << "Todos os testes de seleção passaram com sucesso!\n" << std::endl;
    return 0;
}
