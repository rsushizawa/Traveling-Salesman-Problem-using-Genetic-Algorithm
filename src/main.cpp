#include <exception>
#include <iostream>

#include "cli.hpp"
#include "config.hpp"

int main(int argc, char** argv) {
    Config cfg;
    try {
        cfg = parseArgs(argc, argv);
    } catch (const std::exception& e) {
        std::cerr << "erro: " << e.what() << "\n\n";
        printUsage(std::cerr);
        return 1;
    }

    // Por enquanto só mostra a configuração lida; o solver entra quando estiver pronto.
    writeConfigJson(cfg, std::cout);
}
