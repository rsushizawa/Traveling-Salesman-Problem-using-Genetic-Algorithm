#include "config.hpp"

#include <ostream>

const char* toString(Scenario s) {
    switch (s) {
    case Scenario::Uniform: return "uniform";
    case Scenario::Circle: return "circle";
    }
    return "?";
}

const char* toString(CrossoverType c) {
    switch (c) {
    case CrossoverType::OX: return "ox";
    case CrossoverType::OBX: return "obx";
    case CrossoverType::UniformRepaired: return "uniform";
    }
    return "?";
}

const char* toString(MutationType m) {
    switch (m) {
    case MutationType::Swap: return "swap";
    case MutationType::Insertion: return "insertion";
    case MutationType::Scramble: return "scramble";
    case MutationType::Inversion: return "inversion";
    }
    return "?";
}

const char* toString(SelectionType s) {
    switch (s) {
    case SelectionType::Tournament: return "tournament";
    case SelectionType::Roulette: return "roulette";
    case SelectionType::Alpha: return "alpha";
    case SelectionType::Truncation: return "truncation";
    }
    return "?";
}

namespace {

std::string jsonString(const std::string& s) {
    std::string out = "\"";
    for (char c : s) {
        if (c == '"' || c == '\\') out += '\\';
        out += c;
    }
    return out + '"';
}

}  // namespace

void writeConfigJson(const Config& cfg, std::ostream& out) {
    out << "{\n"
        << "  \"scenario\": \"" << toString(cfg.scenario) << "\",\n"
        << "  \"n\": " << cfg.n << ",\n"
        << "  \"side\": " << cfg.side << ",\n"
        << "  \"radius\": " << cfg.radius << ",\n"
        << "  \"populationSize\": " << cfg.populationSize << ",\n"
        << "  \"crossoverRate\": " << cfg.crossoverRate << ",\n"
        << "  \"mutationRate\": " << cfg.mutationRate << ",\n"
        << "  \"crossover\": \"" << toString(cfg.crossover) << "\",\n"
        << "  \"mutation\": \"" << toString(cfg.mutation) << "\",\n"
        << "  \"selection\": \"" << toString(cfg.selection) << "\",\n"
        << "  \"tournamentSize\": " << cfg.tournamentSize << ",\n"
        << "  \"truncationFraction\": " << cfg.truncationFraction << ",\n"
        << "  \"elitism\": " << cfg.elitism << ",\n"
        << "  \"maxEpochs\": " << cfg.maxEpochs << ",\n"
        << "  \"stagnation\": " << cfg.stagnation << ",\n"
        << "  \"seed\": " << cfg.seed << ",\n"
        << "  \"outDir\": " << jsonString(cfg.outDir) << "\n"
        << "}\n";
}
