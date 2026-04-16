#include "SATSolver.hpp"
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

SATSolver::SATSolver() {
    numVars = 0;
    numClauses = 0;
    std::vector<short int> usedVars; 
    std::vector<Clause> clauses; 
    std::vector<VarInfo> vars;

    solutionExist = 0; 
}

bool SATSolver::DIMACS(std::string& nameFile) {
    std::ifstream file(nameFile);
    std::string comment;

    file >> std::ws;
    
    while (file.peek() == 'c') {
        std::getline(file, comment);
        file >> std::ws;
    }

    file >> comment; // input p
    file >> comment; // input cnf

    file >> numVars;
    file >> numClauses;

    int value;
    int curClause = 0;

    usedVars = std::vector<short int>(numVars + 1);
    clauses = std::vector<Clause>(numClauses);
    vars = std::vector<VarInfo>(numVars + 1);
    clauseIsTrue = std::vector<int>(numClauses);
    varsLeft = std::vector<int>(numClauses);

    file >> std::ws;
    while (file.peek() != EOF) {
        if (file.peek() == 'c') {
            getline(file, comment);
            continue;
        }
        file >> value;
        if (value == 0) {
            curClause++;
        } else {
            clauses[curClause].numOfVars++;
            clauses[curClause].vars.push_back(value);

            varsLeft[curClause]++;

            if (value > 0) {
                vars[curClause].numPosOccur++;
                vars[curClause].posOccur.push_back(curClause);
            } else {
                vars[curClause].numNegOccur++;
                vars[curClause].negOccur.push_back(curClause);
            }
        }
        file >> std::ws;
    }
    return true;
}

/*
1. В цикле кручусь, пока стек не пустой



*/

bool SATSolver::solve() {
    do {
        Var curVar = findNextVar();
        int ok = addVar(curVar);

        if (ok) {
            if (numZeroClauses != 0)
                continue;
            return true;
        }

        backtrack();

        while (!curVar.canChange) { // откат до той переменной, которую можно поменять
            backtrack();
            if (lastVal.size() == 0)
                return false;
            curVar = lastVal.top();
            lastVal.pop();
        }
        curVar.canChange = 0; // пред вариант не подошёл
        curVar.var = -curVar.var;

        ok = addVar(curVar);
        if (ok) {
            if (numZeroClauses != 0)
                continue;
            return true;
        }

        while (!curVar.canChange) {
            backtrack();
            if (lastVal.size() == 0)
                return false;
            curVar = lastVal.top();
            lastVal.pop();
        }
        curVar = findNextVar();
    } while (lastVal.size() > 0);

    return false;
}

bool SATSolver::addVar(Var x) {
    return true;
}

void SATSolver::backtrack() {
    return;
}

Var SATSolver::findNextVar() {
    return Var(1, 1);
}

