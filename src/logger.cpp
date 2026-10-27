#include "logger.hpp"

#include <algorithm>
#include <filesystem>
#include <iomanip>
#include <limits>
#include <stdexcept>

namespace {

std::ofstream openFile(const std::filesystem::path& path) {
    std::ofstream out(path);
    if (!out) throw std::runtime_error("não foi possível criar " + path.string());
    return out;
}

}  // namespace

Logger::Logger(const Config& cfg, const std::vector<Point>& points)
    : routeEvery_(cfg.routeEvery),
      saveRoutes_(cfg.saveRoutes),
      bestSoFar_(std::numeric_limits<double>::infinity()) {
    const std::filesystem::path dir = cfg.outDir;
    std::filesystem::create_directories(dir);

    std::ofstream config = openFile(dir / "config.json");
    writeConfigJson(cfg, config);

    std::ofstream pointsCsv = openFile(dir / "points.csv");
    pointsCsv << std::fixed << std::setprecision(6) << "id,x,y\n";
    for (std::size_t i = 0; i < points.size(); ++i) {
        pointsCsv << i << ',' << points[i].x << ',' << points[i].y << '\n';
    }

    // '\n' em vez de std::endl: o endl faz flush a cada linha.
    stats_ = openFile(dir / "stats.csv");
    stats_ << std::fixed << std::setprecision(4) << "epoch,best,mean,worst,elapsed_ms\n";

    routes_ = openFile(dir / "routes.csv");
    routes_ << "epoch,route\n";

    start_ = std::chrono::steady_clock::now();
}

void Logger::logEpoch(int epoch, const Population& pop) {
    const auto byLength = [](const Individual& a, const Individual& b) {
        return a.length < b.length;
    };
    const auto [best, worst] = std::minmax_element(pop.begin(), pop.end(), byLength);

    double sum = 0.0;
    for (const Individual& ind : pop) sum += ind.length;

    const double elapsedMs =
        std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start_)
            .count();

    stats_ << epoch << ',' << best->length << ',' << sum / pop.size() << ',' << worst->length
           << ',' << elapsedMs << '\n';

    const bool improved = best->length < bestSoFar_;
    if (saveRoutes_ && (improved || (routeEvery_ > 0 && epoch % routeEvery_ == 0))) {
        routes_ << epoch << ',';
        for (std::size_t k = 0; k < best->route.size(); ++k) {
            if (k > 0) routes_ << ' ';
            routes_ << best->route[k];
        }
        routes_ << '\n';
    }
    bestSoFar_ = std::min(bestSoFar_, best->length);
}
