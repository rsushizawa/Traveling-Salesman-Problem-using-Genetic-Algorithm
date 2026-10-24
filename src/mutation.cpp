#include "mutation.hpp"
#include <algorithm>
#include <random>

void swapMutation(Route &r, Rng &rng) {
    if (r.size() < 2) return; 
    std::uniform_int_distribution<size_t> dist1(0, r.size() - 1);
    std::uniform_int_distribution<size_t> dist2(0, r.size() - 2);
    
    size_t pos = dist1(rng);
    size_t pos2 = dist2(rng);
    if (pos2 >= pos) {
        pos2++;
    }

    std::swap(r[pos], r[pos2]);
}

void insertionMutation(Route &r, Rng &rng){
    if (r.size() < 2) return;

    std::uniform_int_distribution<size_t> dist1(0, r.size() - 1);
    std::uniform_int_distribution<size_t> dist2(0, r.size() - 2);

    size_t pos = dist1(rng);
    size_t newPos = dist2(rng);
    if (newPos >= pos) {
        newPos++;
    }

    if (pos < newPos) {                                                                                 
        // Rotaciona para a esquerda no intervalo [pos, newPos]                                         
        std::rotate(r.begin() + pos, r.begin() + pos + 1, r.begin() + newPos + 1);                      
    } else {                                                                                            
        // Rotaciona para a direita no intervalo [newPos, pos]                                          
        std::rotate(r.begin() + newPos, r.begin() + pos, r.begin() + pos + 1);                          
    }
}

void scrambleMutation(Route &r, Rng &rng){
    if (r.size() < 2) return;
    std::uniform_int_distribution<size_t> dist1(0, r.size() - 1);
    std::uniform_int_distribution<size_t> dist2(0, r.size() - 2);
    
    size_t cut1 = dist1(rng);
    size_t cut2 = dist2(rng);
    if (cut2 >= cut1) {
        cut2++;
    }

    if (cut1 > cut2) std::swap(cut1, cut2);

    std::shuffle(r.begin() + cut1, r.begin() + cut2 + 1, rng);
}

void inversionMutation(Route &r, Rng &rng){
    if (r.size() < 2) return; 
    std::uniform_int_distribution<size_t> dist1(0, r.size() - 1);
    std::uniform_int_distribution<size_t> dist2(0, r.size() - 2);
    
    size_t cut1 = dist1(rng);
    size_t cut2 = dist2(rng);
    if (cut2 >= cut1) {
        cut2++;
    }

    if (cut1 > cut2) std::swap(cut1, cut2);

    std::reverse(r.begin() + cut1, r.begin() + cut2 + 1);
}