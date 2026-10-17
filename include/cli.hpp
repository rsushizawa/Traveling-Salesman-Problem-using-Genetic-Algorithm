#pragma once

#include <iosfwd>

#include "config.hpp"

Config parseArgs(int argc, char **argv);

void printUsage(std::ostream &out);
