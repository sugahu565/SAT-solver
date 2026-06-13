#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include "SATSolver.hpp"

std::unique_ptr<SATSolver> makeSolver(bool useJWH) {
    if (useJWH)
        return std::make_unique<SATSolverJWH>();
    return std::make_unique<SATSolverNaive>();
}

int main(int argc, char* argv[]) {
    bool useJWH = false;
    bool printHeuristic = false;
    std::vector<std::string> files;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-jwh") {
            useJWH = true;
        } else if (arg == "--print-heuristic") {
            printHeuristic = true;
        } else if (!arg.empty() && arg[0] == '-') {
            std::cerr << "Unknown option: " << arg << '\n';
            return 1;
        } else {
            files.push_back(arg);
        }
    }

    if (printHeuristic) {
        std::cout << makeSolver(useJWH)->heuristicName() << '\n';
        if (files.empty())
            return 0;
    }

    if (files.empty()) {
        std::cout << "Usage: " << argv[0]
                  << " [-jwh] [--print-heuristic] <file.cnf> [file.cnf ...]" << '\n';
        return 1;
    }

    for (std::string& nameFile : files) {
        std::unique_ptr<SATSolver> solver = makeSolver(useJWH);

        if (!solver->dimacs(nameFile)) {
            std::cout << "Couldn't read or parse " << nameFile << '\n';
            return 1;
        }

        if (solver->solve())
            std::cout << "SAT" << '\n';
        else
            std::cout << "UNSAT" << '\n';
    }

    return 0;
}
