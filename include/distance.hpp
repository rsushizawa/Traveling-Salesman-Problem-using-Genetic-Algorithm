#pragma once

#include <cstddef>
#include <vector>

#include "types.hpp"

// Matriz de adjacência não direcionada com pesos (distância euclidiana),
// calculada uma vez. Armazenada contígua com índice i*n + j.
class DistanceMatrix {
public:
    explicit DistanceMatrix(const std::vector<Point>& points);

    double operator()(int i, int j) const {
        return data_[static_cast<std::size_t>(i) * n_ + j];
    }

    int size() const { return n_; }

private:
    int n_;
    std::vector<double> data_;
};

// Comprimento total da rota, incluindo a aresta de volta (último -> primeiro).
// É a função de aptidão: menor é melhor.
double routeLength(const Route& route, const DistanceMatrix& dist);
