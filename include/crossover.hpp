#pragma once

#include "types.hpp"

// Contrato de todo crossover:
//  - os pais são permutações válidas de 0..n-1 e não são alterados;
//  - o filho retornado é uma permutação válida de 0..n-1;
//  - toda aleatoriedade vem de rng;
//  - não calcula fitness.

// Order Crossover (OX): copia um trecho de a e completa na ordem de b.
Route orderedCrossover(const Route& a, const Route& b, Rng& rng);

// Order-Based Crossover (OBX): posições sorteadas seguem a ordem relativa de b.
Route orderBasedCrossover(const Route& a, const Route& b, Rng& rng);

// Crossover uniforme seguido de reparo das cidades repetidas/faltantes.
Route uniformCrossoverRepaired(const Route& a, const Route& b, Rng& rng);
