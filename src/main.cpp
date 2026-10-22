#include <exception>
#include <iostream>
#include <vector>

#include "cli.hpp"
#include "config.hpp"
#include "distance.hpp"
#include "logger.hpp"
#include "points.hpp"
#include "types.hpp"

int main(int argc, char** argv) {
    Config cfg;
    try {
        cfg = parseArgs(argc, argv);
    } catch (const std::exception& e) {
        std::cerr << "erro: " << e.what() << "\n\n";
        printUsage(std::cerr);
        return 1;
    }

    try {
        // Rng separado para a instância: trocar a seed do GA não muda o mapa.
        Rng instanceRng(cfg.instanceSeed);
        const std::vector<Point> points =
            cfg.scenario == Scenario::Uniform
                ? uniformPoints(cfg.n, cfg.side, instanceRng)
                : circlePoints(cfg.n, cfg.radius, cfg.radius, cfg.radius, instanceRng);

        const DistanceMatrix dist(points);
        Logger log(cfg, points);
        Rng rng(cfg.seed);

        // O solver entra aqui quando estiver pronto:
        // Individual best = Solver(cfg, dist, rng).run(log);
    } catch (const std::exception& e) {
        std::cerr << "erro: " << e.what() << "\n";
        return 1;
    }
}
