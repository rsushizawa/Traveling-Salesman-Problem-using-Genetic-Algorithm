#pragma once

#include <random>
#include <vector>

// Tipos sem módulo dono, compartilhados por todos os operadores.

struct Point {
    double x;
    double y;
};

// Permutação de 0..n-1: a ordem em que as cidades são visitadas.
using Route = std::vector<int>;

// Único gerador de aleatoriedade; criado a partir de cfg.seed e passado por referência.
using Rng = std::mt19937_64;

struct Individual {
    Route route;
    double length = 0.0;  // cache do comprimento da rota; quem atualiza é o solver
};

using Population = std::vector<Individual>;
