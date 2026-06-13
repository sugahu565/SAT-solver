#include <iostream>
#include <string>
#include <cstring>
#include <memory>
#include "SATSolver.hpp"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <file.cnf> [-jwh]" << '\n';
        return 1;
    }

    std::string nameFile = argv[1];
    bool useJWH = false;
    if (argc >= 3 && std::strcmp(argv[2], "-jwh") == 0) {
        useJWH = true;
    }

    std::unique_ptr<SATSolver> solver;
    if (useJWH) {
        solver = std::make_unique<SATSolverJWH>();
    } else {
        solver = std::make_unique<SATSolverNaive>();
    }

    if (!solver->dimacs(nameFile)) {
        std::cout << "Couldn't read or parse " << nameFile << '\n';
        return 1;
    }

    if (solver->solve())
        std::cout << "SAT" << std::endl;
    else
        std::cout << "UNSAT" << std::endl;
    return 0;
}

