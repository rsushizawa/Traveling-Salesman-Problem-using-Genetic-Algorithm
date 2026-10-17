#pragma once

#include "types.hpp"

// Contrato de toda mutação:
//  - altera r no lugar, que continua sendo uma permutação válida de 0..n-1;
//  - toda aleatoriedade vem de rng;
//  - não calcula fitness;
//  - a decisão de mutar (taxa) é do solver, não do operador.

// Troca duas cidades de posição.
void swapMutation(Route& r, Rng& rng);

// Remove uma cidade e a reinsere em outra posição.
void insertionMutation(Route& r, Rng& rng);

// Embaralha um trecho da rota.
void scrambleMutation(Route& r, Rng& rng);

// Inverte um trecho da rota (movimento 2-opt).
void inversionMutation(Route& r, Rng& rng);
