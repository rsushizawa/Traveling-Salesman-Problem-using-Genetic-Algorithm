#include "crossover.hpp"
#include <algorithm>
#include <cstddef>
#include <random>
#include <vector>

Route orderedCrossover(const Route &a, const Route &b, Rng &rng) {
    // Tamanho da rota
    const size_t n = a.size();
    if (n <= 1) return a;

    std::uniform_int_distribution<size_t> dist(0, n - 1);
    size_t cut1 = dist(rng);
    size_t cut2 = dist(rng);
    if (cut1 > cut2) std::swap(cut1, cut2);

    // Criação da rota filho
    Route child(n, -1);
    // Vetor booleano, verifica se o indice (cidade) está presente no filho
    std::vector<bool> in_child(n, false);

    // 1. Copia o segmento [cut1, cut2] do pai a
    for (size_t i = cut1; i <= cut2; ++i) {
        child[i] = a[i];
        in_child[a[i]] = true;
    }

    // 2. Preenche o restante preservando a ordem circular a partir de cut2 + 1
    size_t child_idx = (cut2 + 1) % n;
    size_t parent_idx = (cut2 + 1) % n;

    for (size_t i = 0; i < n; ++i) {
        int city = b[parent_idx];
        // Caso gene não seja repetido, copia e vai para o próxima posição
        if (!in_child[city]) {
            child[child_idx] = city;
            // Passa para próxima posição no child
            child_idx = (child_idx + 1) % n;
        }
        // Pega próximo gene do pai b
        parent_idx = (parent_idx + 1) % n;
    }

    return child;
}

Route orderBasedCrossover(const Route &a, const Route &b, Rng &rng) {
    const size_t n = a.size();
    if (n <= 1) return a;

    // Seleciona posições de 'a' com probabilidade 0.5 (garante ao menos 1)
    std::bernoulli_distribution coin(0.5);
    std::vector<bool> selected(n, false);
    size_t selected_count = 0;

    for (size_t i = 0; i < n; ++i) {
        if (coin(rng)) {
            selected[i] = true;
            selected_count++;
        }
    }

    // Se nenhuma ou todas forem selecionadas, escolhemos uma aleatória para garantir crossover válido
    if (selected_count == 0 || selected_count == n) {
        std::uniform_int_distribution<size_t> dist(0, n - 1);
        selected.assign(n, false);
        selected[dist(rng)] = true;
    } 

    // Identifica quais cidades foram selecionadas na rota A para serem preenchidas
    std::vector<bool> in_selected(n, false);
    for (size_t i = 0; i < n; ++i) {
        if (selected[i]) {
            in_selected[a[i]] = true;
        }
    }

    // Filho começa idêntico ao pai 'a'
    Route child = a;

    // Preenche as posições selecionadas na ordem relativa em que aparecem no pai 'b'
    size_t child_idx = 0;
    for (size_t i = 0; i < n; ++i) {
        int city = b[i];
        if (in_selected[city]) {
            while (!selected[child_idx]) {
                child_idx++;
            }
            child[child_idx] = city;
            child_idx++;
        }
    }

    return child;
}

Route uniformCrossoverRepaired(const Route &a, const Route &b, Rng &rng) {
    // Tamanho da rota (n)
    const size_t n = a.size();
    if (n <= 1) return a;
    
    // Seletor para decidir entre o gene de um dos pais
    std::bernoulli_distribution coin(0.5);

    Route child(n, -1);
    std::vector<bool> present(n, false);
    
    // Vetor com as posições vazias
    std::vector<size_t> empty_positions;
    empty_positions.reserve(n);

    // 1: Seleção uniforme gene a gene dos pais sem introduzir duplicatas
    for (size_t i = 0; i < n; ++i) {
        // Escolhe entre a e b para pos i
        int gene = coin(rng) ? a[i] : b[i];
        // Verifica repetição
        if (!present[gene]) {
            child[i] = gene;
            present[gene] = true;
        } else {
            empty_positions.push_back(i);
        }
    }

    // 2: Reparo preenchendo as cidades faltantes
    if (!empty_positions.empty()) {
        std::vector<int> missing;
        missing.reserve(empty_positions.size());

        // Descobre as cidades faltantes
        for (int city = 0; city < static_cast<int>(n); ++city) {
            if (!present[city]) {
                missing.push_back(city);
            }
        }

        std::shuffle(missing.begin(), missing.end(), rng);

        // Preenche posições vazias com cidades faltantes
        for (size_t i = 0; i < empty_positions.size(); ++i) {
            child[empty_positions[i]] = missing[i];
        }
    }

    return child;
}