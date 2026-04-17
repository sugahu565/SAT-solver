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


bool SATSolver::solve() {
    Var curVar = findNextVar();
    do {
        int ok = addVar(curVar);

        if (ok) {
            if (numZeroClauses != 0) {
                curVar = findNextVar();
                continue;
            }
            return true;
        }

        backtrack();

        if (curVar.canChange) {
            curVar.canChange = 0;
            curVar.var = -curVar.var;
            continue;
        }

        while (!curVar.canChange) { // откат до той переменной, которую можно поменять
            if (lastVal.size() == 0)
                return false;
            curVar = lastVal.top();
            backtrack();
        }
        // откатилась
        curVar.canChange = 0;
        curVar.var = -curVar.var;
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
    Var needReturn;
    if (single.size()) {
        needReturn.var = single.back();
        needReturn.canChange = 0;
        return needReturn;
    }
    for (int i = 1; i <= numVars; ++i) {
        if (usedVars[i] == 0) {
            needReturn.var = i;
            needReturn.canChange = 1;
            return needReturn;
        }
    }
    needReturn.var = -1; // в теории, до сюда не дойдёт
    return needReturn;
}

