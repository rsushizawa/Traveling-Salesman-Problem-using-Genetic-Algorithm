#include "distance.hpp"

#include <cmath>

DistanceMatrix::DistanceMatrix(const std::vector<Point>& points)
    : n_(static_cast<int>(points.size())) {
    if (n_ > kMaxCachedCities) {
        points_ = points;
        return;
    }
    data_.assign(points.size() * points.size(), 0.0);
    // Simétrica: calcula só i < j e espelha. A diagonal fica em 0.
    for (int i = 0; i < n_; ++i) {
        for (int j = i + 1; j < n_; ++j) {
            const double d = std::hypot(points[i].x - points[j].x, points[i].y - points[j].y);
            data_[static_cast<std::size_t>(i) * n_ + j] = d;
            data_[static_cast<std::size_t>(j) * n_ + i] = d;
        }
    }
}

double routeLength(const Route& route, const DistanceMatrix& dist) {
    if (route.empty()) return 0.0;

    double total = 0.0;
    for (std::size_t k = 0; k + 1 < route.size(); ++k) {
        total += dist(route[k], route[k + 1]);
    }
    return total + dist(route.back(), route.front());
}
