#pragma once

#include <chrono>
#include <fstream>
#include <vector>

#include "config.hpp"
#include "types.hpp"

// Grava a pasta de uma execução (cfg.outDir), lida depois pelos scripts Python:
//   config.json  configuração completa
//   points.csv   id,x,y
//   stats.csv    epoch,best,mean,worst,elapsed_ms   (uma linha por época)
//   routes.csv   epoch,route                        (cidades separadas por espaço)
class Logger {
public:
    // Cria a pasta e grava config.json e points.csv.
    // Lança std::runtime_error se não conseguir escrever.
    Logger(const Config& cfg, const std::vector<Point>& points);

    // Registra uma época. Exige pop não vazia e com length já calculado.
    // A rota do melhor vai para routes.csv quando ele melhora ou a cada cfg.routeEvery épocas.
    void logEpoch(int epoch, const Population& pop);

private:
    std::ofstream stats_;
    std::ofstream routes_;
    int routeEvery_;
    double bestSoFar_;
    std::chrono::steady_clock::time_point start_;
};
