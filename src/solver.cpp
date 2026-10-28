#include "solver.hpp"

#include <algorithm>
#include <numeric>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

#include "crossover.hpp"
#include "logger.hpp"
#include "mutation.hpp"
#include "selection.hpp"

namespace {

void require(bool ok, const std::string& message) {
    if (!ok) throw std::invalid_argument(message);
}

void validate(const Config& cfg, const DistanceMatrix& dist) {
    require(dist.size() == cfg.n, "matriz de distâncias não tem n cidades");
    require(cfg.populationSize >= 2, "pop: precisa ser pelo menos 2");
    require(cfg.crossoverRate >= 0.0 && cfg.crossoverRate <= 1.0, "cross-rate: precisa estar em [0, 1]");
    require(cfg.mutationRate >= 0.0 && cfg.mutationRate <= 1.0, "mut: precisa estar em [0, 1]");
    require(cfg.elitism >= 0 && cfg.elitism < cfg.populationSize, "elitism: precisa estar em [0, pop)");
    require(cfg.tournamentSize >= 1, "tournament: precisa ser pelo menos 1");
    require(cfg.truncationFraction > 0.0 && cfg.truncationFraction <= 1.0, "truncation: precisa estar em (0, 1]");
    require(cfg.maxEpochs >= 1, "max-epochs: precisa ser pelo menos 1");
    require(cfg.stagnation >= 0, "stagnation: não pode ser negativo");
}

}  // namespace

CrossoverFn makeCrossover(const Config& cfg) {
    switch (cfg.crossover) {
    case CrossoverType::OX: return orderedCrossover;
    case CrossoverType::OBX: return orderBasedCrossover;
    case CrossoverType::UniformRepaired: return uniformCrossoverRepaired;
    }
    throw std::invalid_argument("crossover desconhecido");
}

MutationFn makeMutation(const Config& cfg) {
    switch (cfg.mutation) {
    case MutationType::Swap: return swapMutation;
    case MutationType::Insertion: return insertionMutation;
    case MutationType::Scramble: return scrambleMutation;
    case MutationType::Inversion: return inversionMutation;
    }
    throw std::invalid_argument("mutação desconhecida");
}

SelectionFn makeSelection(const Config& cfg) {
    switch (cfg.selection) {
    case SelectionType::Tournament:
        return [k = cfg.tournamentSize](const Population& pop, Rng& rng) {
            return tournamentSelect(pop, k, rng);
        };
    case SelectionType::Roulette: return rouletteSelect;
    case SelectionType::Alpha: return alphaSelect;
    case SelectionType::Truncation:
        return [f = cfg.truncationFraction](const Population& pop, Rng& rng) {
            return truncationSelect(pop, f, rng);
        };
    }
    throw std::invalid_argument("seleção desconhecida");
}

Solver::Solver(const Config& cfg, const DistanceMatrix& dist, Rng& rng)
    : cfg_(cfg), dist_(dist), rng_(rng) {
    validate(cfg_, dist_);
    crossover_ = makeCrossover(cfg_);
    mutate_ = makeMutation(cfg_);
    select_ = makeSelection(cfg_);
}

Individual Solver::run(Logger& log) {
    std::uniform_real_distribution<double> coin(0.0, 1.0);

    // Época 0 é a população inicial: permutações aleatórias.
    Population pop(cfg_.populationSize);
    for (Individual& ind : pop) {
        ind.route.resize(cfg_.n);
        std::iota(ind.route.begin(), ind.route.end(), 0);
        std::shuffle(ind.route.begin(), ind.route.end(), rng_);
        ind.length = routeLength(ind.route, dist_);
    }

    const auto byLength = [](const Individual& a, const Individual& b) {
        return a.length < b.length;
    };

    Individual best = *std::min_element(pop.begin(), pop.end(), byLength);
    int sinceImproved = 0;
    std::vector<int> order(pop.size());
    Population next;
    next.reserve(pop.size());

    for (int epoch = 0;; ++epoch) {
        log.logEpoch(epoch, pop);

        const Individual& epochBest = *std::min_element(pop.begin(), pop.end(), byLength);
        if (epochBest.length < best.length) {
            best = epochBest;
            sinceImproved = 0;
        } else if (epoch > 0) {
            ++sinceImproved;
        }

        if (epoch + 1 >= cfg_.maxEpochs) break;
        if (cfg_.stagnation > 0 && sinceImproved >= cfg_.stagnation) break;

        // Elitismo: os `elitism` melhores passam sem alteração.
        next.clear();
        std::iota(order.begin(), order.end(), 0);
        std::partial_sort(order.begin(), order.begin() + cfg_.elitism, order.end(),[&](int a, int b) { return pop[a].length < pop[b].length; });
        for (int k = 0; k < cfg_.elitism; ++k) next.push_back(pop[order[k]]);

        // Seleção devolve índices em pop, que só é trocada depois que next está completa.
        while (static_cast<int>(next.size()) < cfg_.populationSize) {
            const Route& a = pop[select_(pop, rng_)].route;
            Individual child;
            if (coin(rng_) < cfg_.crossoverRate) {
                const Route& b = pop[select_(pop, rng_)].route;
                child.route = crossover_(a, b, rng_);
            } else {
                child.route = a;
            }
            if (coin(rng_) < cfg_.mutationRate) mutate_(child.route, rng_);
            child.length = routeLength(child.route, dist_);
            next.push_back(std::move(child));
        }
        std::swap(pop, next);
    }

    return best;
}
