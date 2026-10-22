#include "cli.hpp"

#include <cstdlib>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>

namespace {

const std::map<std::string, Scenario> scenarioNames = {
    {"uniform", Scenario::Uniform},
    {"circle", Scenario::Circle},
};

const std::map<std::string, CrossoverType> crossoverNames = {
    {"ox", CrossoverType::OX},
    {"obx", CrossoverType::OBX},
    {"uniform", CrossoverType::UniformRepaired},
};

const std::map<std::string, MutationType> mutationNames = {
    {"swap", MutationType::Swap},
    {"insertion", MutationType::Insertion},
    {"scramble", MutationType::Scramble},
    {"inversion", MutationType::Inversion},
};

const std::map<std::string, SelectionType> selectionNames = {
    {"tournament", SelectionType::Tournament},
    {"roulette", SelectionType::Roulette},
    {"alpha", SelectionType::Alpha},
    {"truncation", SelectionType::Truncation},
};

// .at() lança std::out_of_range para nomes inválidos; relançamos com as opções
// válidas.
template <typename Enum>
Enum parseEnum(const std::map<std::string, Enum> &names,
               const std::string &flag, const std::string &value) {
  try {
    return names.at(value);
  } catch (const std::out_of_range &) {
    std::string valid;
    for (const auto &[name, _] : names)
      valid += " " + name;
    throw std::invalid_argument(flag + ": valor inválido '" + value +
                                "' (opções:" + valid + ")");
  }
}

// std::stoi/stod aceitam lixo no final ("12abc"); `pos` garante que o valor
// inteiro foi lido.
int parseInt(const std::string &flag, const std::string &value) {
  try {
    std::size_t pos = 0;
    int result = std::stoi(value, &pos);
    if (pos == value.size())
      return result;
  } catch (const std::logic_error &) {
  }
  throw std::invalid_argument(flag + ": esperava um inteiro, recebeu '" +
                              value + "'");
}

double parseDouble(const std::string &flag, const std::string &value) {
  try {
    std::size_t pos = 0;
    double result = std::stod(value, &pos);
    if (pos == value.size())
      return result;
  } catch (const std::logic_error &) {
  }
  throw std::invalid_argument(flag + ": esperava um número, recebeu '" + value +
                              "'");
}

std::uint64_t parseSeed(const std::string &flag, const std::string &value) {
  try {
    std::size_t pos = 0;
    std::uint64_t result = std::stoull(value, &pos);
    if (pos == value.size() && value[0] != '-')
      return result;
  } catch (const std::logic_error &) {
  }
  throw std::invalid_argument(
      flag + ": esperava um inteiro não negativo, recebeu '" + value + "'");
}

} // namespace

void printUsage(std::ostream &out) {
  out << "Uso: ga [opções]\n"
         "\n"
         "Problema:\n"
         "  --scenario uniform|circle   distribuição dos pontos\n"
         "  --n N                       número de cidades\n"
         "  --side L                    lado do quadrado (uniform)\n"
         "  --radius R                  raio do círculo (circle)\n"
         "\n"
         "Algoritmo genético:\n"
         "  --pop N                     tamanho da população\n"
         "  --cross-rate P              taxa de crossover\n"
         "  --mut P                     taxa de mutação\n"
         "  --cross ox|obx|uniform\n"
         "  --mutation swap|insertion|scramble|inversion\n"
         "  --sel tournament|roulette|alpha|truncation\n"
         "  --tournament K              tamanho do torneio\n"
         "  --truncation F              fração dos melhores no truncamento\n"
         "  --elitism N                 melhores que passam direto\n"
         "\n"
         "Parada:\n"
         "  --max-epochs N\n"
         "  --stagnation N              épocas sem melhora (0 desativa)\n"
         "\n"
         "Execução:\n"
         "  --instance-seed S           seed dos pontos (mesma seed = mesmo mapa)\n"
         "  --seed S                    seed do algoritmo genético\n"
         "  --out DIR                   pasta de saída\n"
         "  --route-every N             grava a rota a cada N épocas, além de\n"
         "                              quando o melhor muda (0 desativa)\n";
}

Config parseArgs(int argc, char **argv) {
  Config cfg;

  for (int i = 1; i < argc; ++i) {
    const std::string flag = argv[i];

    if (flag == "--help" || flag == "-h") {
      printUsage(std::cout);
      std::exit(0);
    }
    if (i + 1 >= argc)
      throw std::invalid_argument(flag + ": faltou o valor");
    const std::string value = argv[++i];

    if (flag == "--scenario")
      cfg.scenario = parseEnum(scenarioNames, flag, value);
    else if (flag == "--n")
      cfg.n = parseInt(flag, value);
    else if (flag == "--side")
      cfg.side = parseDouble(flag, value);
    else if (flag == "--radius")
      cfg.radius = parseDouble(flag, value);
    else if (flag == "--pop")
      cfg.populationSize = parseInt(flag, value);
    else if (flag == "--cross-rate")
      cfg.crossoverRate = parseDouble(flag, value);
    else if (flag == "--mut")
      cfg.mutationRate = parseDouble(flag, value);
    else if (flag == "--cross")
      cfg.crossover = parseEnum(crossoverNames, flag, value);
    else if (flag == "--mutation")
      cfg.mutation = parseEnum(mutationNames, flag, value);
    else if (flag == "--sel")
      cfg.selection = parseEnum(selectionNames, flag, value);
    else if (flag == "--tournament")
      cfg.tournamentSize = parseInt(flag, value);
    else if (flag == "--truncation")
      cfg.truncationFraction = parseDouble(flag, value);
    else if (flag == "--elitism")
      cfg.elitism = parseInt(flag, value);
    else if (flag == "--max-epochs")
      cfg.maxEpochs = parseInt(flag, value);
    else if (flag == "--stagnation")
      cfg.stagnation = parseInt(flag, value);
    else if (flag == "--instance-seed")
      cfg.instanceSeed = parseSeed(flag, value);
    else if (flag == "--seed")
      cfg.seed = parseSeed(flag, value);
    else if (flag == "--out")
      cfg.outDir = value;
    else if (flag == "--route-every")
      cfg.routeEvery = parseInt(flag, value);
    else
      throw std::invalid_argument("opção desconhecida: " + flag);
  }

  return cfg;
}
