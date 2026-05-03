#include "SATSolver.hpp"
#include <cmath>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

SATSolver::SATSolver() {
    numVars = 0;
    numClauses = 0;
    solutionExist = 0;
    pows = std::vector<double>(128);
    for (int i = 0; i < 128; i++)
        pows[i] = std::pow(2.0, -i);
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
    varsInfo = std::vector<VarInfo>(numVars + 1);
    clauseIsTrue = std::vector<int>(numClauses);
    varsLeft = std::vector<int>(numClauses);

    file >> std::ws;
    while (file.peek() != EOF && file.peek() != '%') {
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
                varsInfo[abs(value)].numPosOccur++;
                varsInfo[abs(value)].posOccur.push_back(curClause);
            } else {
                varsInfo[abs(value)].numNegOccur++;
                varsInfo[abs(value)].negOccur.push_back(curClause);
            }
        }
        file >> std::ws;
    }
    numZeroClauses = numClauses;
    return true;
}


bool SATSolver::solve() {
    Var curVar = getNextVarJWH();
    do {
        int ok = addVar(curVar);

        if (ok) {
            if (numZeroClauses != 0) {
                curVar = getNextVarJWH();
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
            if (lastVar.size() == 0)
                return false;
            curVar = lastVar.top();
            backtrack();
        }
        // откатилась
        curVar.canChange = 0;
        curVar.var = -curVar.var;
    } while (lastVar.size() > 0);

    return false;
}

bool SATSolver::addVar(Var x) {
    int curNum = abs(x.var);
    bool pos = x.var > 0;
    int flag = 1;
    std::vector<int>* trueClauses;
    std::vector<int>* otherClauses;
    if (pos) {
        trueClauses = &varsInfo[curNum].posOccur;
        otherClauses = &varsInfo[curNum].negOccur;
    } else {
        trueClauses = &varsInfo[curNum].negOccur;
        otherClauses = &varsInfo[curNum].posOccur;
    }
    for (auto c: *trueClauses) {
        clauseIsTrue[c]++;
        if (clauseIsTrue[c] == 1)
            numZeroClauses--;
        varsLeft[c]--;
    }
    for (auto c: *otherClauses) {
        varsLeft[c]--;
        if (!clauseIsTrue[c] && varsLeft[c] == 1)
            single.push(findNotUsedVar(clauses[c]));
        else if (!clauseIsTrue[c] && varsLeft[c] == 0)
            flag = 0;
    }
    lastVar.push(x);
    usedVars[curNum] = pos ? 1 : -1;
    return flag;
}

void SATSolver::backtrack() {
    while (!single.empty())
        single.pop();
    // Симметрично addVar

    int curNum = abs(lastVar.top().var);
    int pos = (lastVar.top().var > 0);
    std::vector<int>* trueClauses;
    std::vector<int>* otherClauses;
    if (pos) {
        trueClauses = &varsInfo[curNum].posOccur;
        otherClauses = &varsInfo[curNum].negOccur;
    } else {
        trueClauses = &varsInfo[curNum].negOccur;
        otherClauses = &varsInfo[curNum].posOccur;
    }
    for (auto c: *trueClauses) {
        clauseIsTrue[c]--;
        if (clauseIsTrue[c] == 0)
            numZeroClauses++;
        varsLeft[c]++;
    }
    for (auto c: *otherClauses) {
        varsLeft[c]++;
    }
    usedVars[curNum] = 0;
    lastVar.pop();
}

Var SATSolver::getNextVar() {
    Var needReturn;
    while (single.size()) {
        if (usedVars[abs(single.top())] != 0) {
            single.pop();
        } else {
            needReturn.var = single.top();
            needReturn.canChange = 0;
            single.pop();
            return needReturn;
        }
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

Var SATSolver::getNextVarJWH() {
    Var needReturn;
    while (single.size()) {
        if (usedVars[abs(single.top())] != 0) {
            single.pop();
        } else {
            needReturn.var = single.top();
            needReturn.canChange = 0;
            single.pop();
            return needReturn;
        }
    }
    double bestWeight = -1;
    for (int i = 1; i <= numVars; ++i) {
        if (usedVars[i] != 0)
            continue;

        double scorePos = 0.0;
        double scoreNeg = 0.0;


        for (auto c: varsInfo[i].posOccur) {
            if (clauseIsTrue[c] == 0) {
                int len = varsLeft[c];
                double weight = (len < 128) ? pows[len] : 0.0;
                scorePos += weight;
            }
        }

        for (auto c: varsInfo[i].negOccur) {
            if (clauseIsTrue[c] == 0) {
                int len = varsLeft[c];
                double weight = (len < 128) ? pows[len] : 0.0;
                scoreNeg += weight;
            }
        }

        if (scoreNeg + scorePos > bestWeight) {
            needReturn.var = scorePos > scoreNeg ? i : -i;
            needReturn.canChange = 1;
            bestWeight = scorePos + scoreNeg;
        }
    }
    return needReturn;
}

int SATSolver::findNotUsedVar(const Clause& curr) {
    for (auto c: curr.vars) {
        if (usedVars[abs(c)] == 0)
            return c;
    }
    return 0;
}

