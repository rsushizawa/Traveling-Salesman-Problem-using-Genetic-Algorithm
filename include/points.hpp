#pragma once

#include <vector>

#include "types.hpp"

// n pontos uniformes no quadrado [0, side] x [0, side]. Exige n >= 8.
std::vector<Point> uniformPoints(int n, double side, Rng& rng);

// n pontos igualmente espaçados no círculo de centro (cx, cy) e raio r,
// já embaralhados (senão a ordem dos índices seria a solução ótima).
std::vector<Point> circlePoints(int n, double cx, double cy, double r, Rng& rng);

// Comprimento ótimo conhecido para circlePoints: 2·n·r·sin(π/n).
double circleOptimum(int n, double r);
