#include "points.hpp"

#include <cmath>
#include <cstdint>
#include <numbers>
#include <stdexcept>
#include <string>
#include <utility>

namespace {

// std::uniform_real_distribution e std::shuffle não têm algoritmo fixado pelo
// padrão (libstdc++ e libc++ dão resultados diferentes). O mt19937_64 tem, então
// derivamos tudo direto dele: a mesma instanceSeed gera o mesmo mapa em qualquer máquina.

// Real em [0, 1) usando os 53 bits de mantissa do double.
double uniform01(Rng& rng) {
    return static_cast<double>(rng() >> 11) * 0x1.0p-53;
}

// Inteiro em [0, bound) sem viés: descarta a sobra 2^64 mod bound.
std::uint64_t uniformBelow(std::uint64_t bound, Rng& rng) {
    const std::uint64_t threshold = (0 - bound) % bound;
    std::uint64_t x;
    do {
        x = rng();
    } while (x < threshold);
    return x % bound;
}

// Fisher-Yates.
void shuffle(std::vector<Point>& v, Rng& rng) {
    for (std::size_t i = v.size(); i > 1; --i) {
        std::swap(v[i - 1], v[uniformBelow(i, rng)]);
    }
}

void requireMinCities(int n) {
    if (n < 8) {
        throw std::invalid_argument("n: precisa de pelo menos 8 cidades, recebeu " +
                                    std::to_string(n));
    }
}

}  // namespace

std::vector<Point> uniformPoints(int n, double side, Rng& rng) {
    requireMinCities(n);

    std::vector<Point> points(n);
    for (Point& p : points) {
        p.x = side * uniform01(rng);
        p.y = side * uniform01(rng);
    }
    return points;
}

std::vector<Point> circlePoints(int n, double cx, double cy, double r, Rng& rng) {
    requireMinCities(n);

    std::vector<Point> points(n);
    for (int i = 0; i < n; ++i) {
        const double theta = 2.0 * std::numbers::pi * i / n;
        points[i] = {cx + r * std::cos(theta), cy + r * std::sin(theta)};
    }
    shuffle(points, rng);
    return points;
}

double circleOptimum(int n, double r) {
    return 2.0 * n * r * std::sin(std::numbers::pi / n);
}
