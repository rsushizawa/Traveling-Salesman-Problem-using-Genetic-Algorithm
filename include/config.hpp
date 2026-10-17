#pragma once

#include <cstdint>
#include <iosfwd>
#include <string>

// A Config guarda só escolhas (enums e números), para continuar serializável
// no config.json de cada execução. O Solver é quem transforma os enums em funções.

enum class Scenario { Uniform, Circle };
enum class CrossoverType { OX, OBX, UniformRepaired };
enum class MutationType { Swap, Insertion, Scramble, Inversion };
enum class SelectionType { Tournament, Roulette, Alpha, Truncation };

struct Config {
    // Problema
    Scenario scenario = Scenario::Uniform;
    int n = 50;
    double side = 1000.0;   // lado do quadrado (Uniform)
    double radius = 500.0;  // raio do círculo (Circle)

    // Algoritmo genético
    int populationSize = 200;
    double crossoverRate = 0.9;
    double mutationRate = 0.05;
    CrossoverType crossover = CrossoverType::OX;
    MutationType mutation = MutationType::Inversion;
    SelectionType selection = SelectionType::Tournament;
    int tournamentSize = 3;           // só usado por Tournament
    double truncationFraction = 0.5;  // só usado por Truncation
    int elitism = 2;                  // quantos melhores passam direto

    // Parada
    int maxEpochs = 1000;
    int stagnation = 200;  // épocas sem melhora antes de parar; 0 desativa

    // Execução
    std::uint64_t seed = 42;
    std::string outDir = "results/run";
};

// Nome de cada enum, igual ao aceito na linha de comando.
const char* toString(Scenario s);
const char* toString(CrossoverType c);
const char* toString(MutationType m);
const char* toString(SelectionType s);

// Escreve a Config completa como JSON (conteúdo do config.json de cada execução).
void writeConfigJson(const Config& cfg, std::ostream& out);
