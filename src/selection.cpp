#include "selection.hpp"
#include "types.hpp"
#include <algorithm>
#include <cstddef>
#include <random>

int tournamentSelect(const Population &pop, int k, Rng &rng){
    // Cria um vetor com posições, embaralha e seleciona os k primeiros
    std::uniform_int_distribution<size_t> dist(0, pop.size() - 1);                                      
    
    // Sorteia o primeiro participante
    int selected = dist(rng);
    double bestLen = pop[selected].length;

    // Sorteia os demais
    for(int i = 1; i < k; i++){                                                                         
        size_t candidate;                                                                               
        // Evita posições repetidas (para k pequeno, o loop roda quase sempre 1 vez)                                                                                                             
        candidate = dist(rng);                                                      
                                                                                                  
        if(pop[candidate].length < bestLen){                                                            
            selected = candidate;                                                                       
            bestLen = pop[candidate].length;                                                            
        }                                                                                               
    } 

    return selected;
}

int rouletteSelect(const Population &pop, Rng &rng){
    double fitnessSum = 0;

    // Calculamos o fitness total invertendo o tamanho da rota de cada individuo
    for(const auto& ind: pop){
        fitnessSum += 1.0 / ind.length;
    }

    std::uniform_real_distribution<double> distRoulette(0.0, fitnessSum);
    double pointer = distRoulette(rng);

    double totalSum = 0.0;
    size_t ind = 0;
    
    // Escolhe o individuo somando o Fitness até onde o ponteiro parou
    for( ; ind < pop.size() - 1 ; ind++){
        totalSum += 1.0 / pop[ind].length;
        if(totalSum >= pointer){
            break;
        }
    }

    return ind;
}

int alphaSelect(const Population &pop, Rng &rng){
    (void)rng;
    double bestFit;
    int alpha;
    
    bestFit = pop[0].length;
    alpha = 0;

    for(size_t i = 1; i < pop.size(); i++){
        if(pop[i].length < bestFit){
            bestFit = pop[i].length;
            alpha = i;
        }
    }

    return alpha;
}

int truncationSelect(const Population &pop, double fraction, Rng &rng){
    // Garante que pelo menos 1 indivíduo seja selecionado
    size_t limite = std::clamp<size_t>(pop.size() * fraction, 1, pop.size());
    
    // Vetor de índices para não modificar 'pop' (que é const)                                          
    std::vector<size_t> indexes(pop.size());                                                            
    std::iota(indexes.begin(), indexes.end(), 0); 

    // Ordena apenas os melhores até 'limite'                                                           
    std::partial_sort(indexes.begin(), indexes.begin() + limite, indexes.end(),[&pop](size_t a, size_t b) {                                                      
        return pop[a].length < pop[b].length;
    });

    // Sorteia pais unicamente do grupo de elite
    std::uniform_int_distribution<size_t> distElite(0, limite - 1);

    return indexes[distElite(rng)];
}