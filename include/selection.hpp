#pragma once

#include "types.hpp"

// Contrato de toda seleção:
//  - retorna o ÍNDICE do indivíduo em pop, não referência
//    (referências ficam inválidas quando a próxima geração é montada);
//  - compara por Individual::length (menor é melhor), já calculado pelo solver;
//  - toda aleatoriedade vem de rng.

// Torneio entre k indivíduos sorteados; vence o de menor length.
int tournamentSelect(const Population& pop, int k, Rng& rng);

// Roleta com probabilidade proporcional a 1/length.
int rouletteSelect(const Population& pop, Rng& rng);

int alphaSelect(const Population& pop, Rng& rng);

// Sorteia entre a fração `fraction` dos melhores indivíduos.
int truncationSelect(const Population& pop, double fraction, Rng& rng);
