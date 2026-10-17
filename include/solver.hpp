#pragma once

#include <functional>

#include "config.hpp"
#include "distance.hpp"
#include "types.hpp"

class Logger;

// Assinaturas únicas dos operadores. Parâmetros extras (ex.: tamanho do torneio)
// são capturados por lambda nas funções make*.
using CrossoverFn = std::function<Route(const Route&, const Route&, Rng&)>;
using MutationFn = std::function<void(Route&, Rng&)>;
using SelectionFn = std::function<int(const Population&, Rng&)>;

CrossoverFn makeCrossover(const Config& cfg);
MutationFn makeMutation(const Config& cfg);
SelectionFn makeSelection(const Config& cfg);

class Solver {
public:
    // Valida cfg e resolve os operadores uma única vez.
    Solver(const Config& cfg, const DistanceMatrix& dist, Rng& rng);

    // Executa até maxEpochs ou estagnação e retorna o melhor indivíduo.
    Individual run(Logger& log);

private:
    Config cfg_;
    const DistanceMatrix& dist_;
    Rng& rng_;

    CrossoverFn crossover_;
    MutationFn mutate_;
    SelectionFn select_;
};
