#pragma once

#include <cmath>
#include <cstddef>
#include <vector>

#include "types.hpp"

// Matriz de adjacência não direcionada com pesos (distância euclidiana),
// calculada uma vez. Armazenada contígua com índice i*n + j.
// Acima de kMaxCachedCities a matriz não cabe na memória (n² doubles: 1M cidades
// dariam 8 TB), então a distância é calculada na hora a partir dos pontos.
// Mesma fórmula (std::hypot) nos dois casos, então o resultado é idêntico.
class DistanceMatrix {
public:
    static constexpr int kMaxCachedCities = 4096;

    explicit DistanceMatrix(const std::vector<Point>& points);

    double operator()(int i, int j) const {
        if (data_.empty()) {
            return std::hypot(points_[i].x - points_[j].x, points_[i].y - points_[j].y);
        }
        return data_[static_cast<std::size_t>(i) * n_ + j];
    }

    int size() const { return n_; }

private:
    int n_;
    std::vector<Point> points_;  // só usado acima de kMaxCachedCities
    std::vector<double> data_;
};

// Comprimento total da rota, incluindo a aresta de volta (último -> primeiro).
// É a função de aptidão: menor é melhor.
double routeLength(const Route& route, const DistanceMatrix& dist);
