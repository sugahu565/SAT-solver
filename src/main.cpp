#include <iostream>
#include "SATSolver.hpp"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Pass the file name" << '\n';
        return 1;
    }

    std::string nameFile = argv[1];
    SATSolver solver;

    if (!solver.DIMACS(nameFile)) {
        std::cout << "Couldn't read or parse " << nameFile << '\n';
        return 1;
    }

    if (solver.solve())
        std::cout << "SAT" << std::endl;
    else
        std::cout << "UNSAT" << std::endl;
    return 0;
}

