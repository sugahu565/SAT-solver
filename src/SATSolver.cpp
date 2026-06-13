#include "SATSolver.hpp"
#include <cmath>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

SATSolver::SATSolver() {
    numVars = 0;
    numClauses = 0;
    numZeroClauses = 0;
    solutionExist = 0;
}

Solution SATSolver::getSolution() {
    Solution s;
    s.solutionExist = solutionExist;
    // Dummy return, actual variable assignment extraction is omitted 
    // unless needed by another part of the system.
    return s;
}

bool SATSolver::dimacs(std::string& nameFile) {
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

    usedVars = std::vector<short int>(numVars + 1, 0);
    clauses = std::vector<Clause>(numClauses);
    varsInfo = std::vector<VarInfo>(numVars + 1);
    clauseIsTrue = std::vector<int>(numClauses, 0);
    varsLeft = std::vector<int>(numClauses, 0);

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

bool SATSolverNaive::solve() {
    if (numZeroClauses == 0) {
        solutionExist = 1;
        return true;
    }

    Var curVar = getNextVar();
    if (curVar.var == 0)
        return false;

    do {
        bool ok = addVar(curVar);

        if (ok) {
            if (numZeroClauses != 0) {
                curVar = getNextVar();
                if (curVar.var != 0)
                    continue;
                curVar = lastVar.top();
            } else {
                solutionExist = 1;
                return true;
            }
        }

        backtrack();

        if (curVar.canChange) {
            curVar.canChange = 0;
            curVar.var = -curVar.var;
            continue;
        }

        while (!curVar.canChange) { // откат до той переменной, которую можно поменять
            if (lastVar.empty())
                return false;
            curVar = lastVar.top();
            backtrack();
        }
        // откатилась
        curVar.canChange = 0;
        curVar.var = -curVar.var;
    } while (true);
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
    usedVars[curNum] = pos ? 1 : -1;
    for (auto c: *trueClauses) {
        clauseIsTrue[c]++;
        if (clauseIsTrue[c] == 1)
            numZeroClauses--;
        varsLeft[c]--;
    }
    for (auto c: *otherClauses) {
        varsLeft[c]--;
        if (!clauseIsTrue[c] && varsLeft[c] == 1)
            single.push(c);
        else if (!clauseIsTrue[c] && varsLeft[c] == 0)
            flag = 0;
    }
    lastVar.push(x);
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
    usedVars[curNum] = 0;
    for (auto c: *trueClauses) {
        clauseIsTrue[c]--;
        if (clauseIsTrue[c] == 0)
            numZeroClauses++;
        varsLeft[c]++;
    }
    for (auto c: *otherClauses) {
        varsLeft[c]++;
    }
    lastVar.pop();
}

int SATSolver::findNotUsedVar(const Clause& curr) {
    for (auto c: curr.vars) {
        if (usedVars[abs(c)] == 0)
            return c;
    }
    return 0;
}

// --- SATSolverNaive ---
Var SATSolverNaive::getNextVar() {
    Var needReturn{0, 0};
    while (!single.empty()) {
        int clause = single.top();
        single.pop();
        if (!clauseIsTrue[clause] && varsLeft[clause] == 1) {
            needReturn.var = findNotUsedVar(clauses[clause]);
            if (needReturn.var == 0)
                continue;
            needReturn.canChange = 0;
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
    needReturn.var = 0;
    return needReturn;
}

// --- SATSolverJWH ---
SATSolverJWH::SATSolverJWH() : SATSolver() {
    pows = std::vector<double>(128);
    for (int i = 0; i < 128; i++)
        pows[i] = std::pow(2.0, -i);
}

bool SATSolverJWH::solve() {
    initHeuristic();

    if (numZeroClauses == 0) {
        solutionExist = 1;
        return true;
    }

    Var curVar = getNextVar();
    if (curVar.var == 0)
        return false;

    do {
        bool ok = addVar(curVar);

        if (ok) {
            if (numZeroClauses != 0) {
                curVar = getNextVar();
                if (curVar.var != 0)
                    continue;
                curVar = lastVar.top();
            } else {
                solutionExist = 1;
                return true;
            }
        }

        backtrack();

        if (curVar.canChange) {
            curVar.canChange = 0;
            curVar.var = -curVar.var;
            continue;
        }

        while (!curVar.canChange) { // откат до той переменной, которую можно поменять
            if (lastVar.empty())
                return false;
            curVar = lastVar.top();
            backtrack();
        }
        // откатилась
        curVar.canChange = 0;
        curVar.var = -curVar.var;
    } while (true);
}

void SATSolverJWH::initHeuristic() {
    weightPos.assign(numVars + 1, 0.0);
    weightNeg.assign(numVars + 1, 0.0);
    for (int i = 1; i <= numVars; ++i) {
        for (auto c : varsInfo[i].posOccur) {
            int len = clauses[c].numOfVars;
            weightPos[i] += (len < 128) ? pows[len] : 0.0;
        }
        for (auto c : varsInfo[i].negOccur) {
            int len = clauses[c].numOfVars;
            weightNeg[i] += (len < 128) ? pows[len] : 0.0;
        }
    }
}

bool SATSolverJWH::addVar(Var x) {
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
    usedVars[curNum] = pos ? 1 : -1;

    for (auto c: *trueClauses) {
        clauseIsTrue[c]++;
        if (clauseIsTrue[c] == 1) {
            numZeroClauses--;
            // Клауза стала удовлетворенной, вычитаем её вес из свободных переменных
            int len = varsLeft[c];
            double w = (len < 128) ? pows[len] : 0.0;
            for (int v_in_c : clauses[c].vars) {
                int abs_v = abs(v_in_c);
                if (usedVars[abs_v] == 0) {
                    if (v_in_c > 0) weightPos[abs_v] -= w;
                    else            weightNeg[abs_v] -= w;
                }
            }
        }
        varsLeft[c]--;
    }

    for (auto c: *otherClauses) {
        varsLeft[c]--;
        int len = varsLeft[c];

        if (clauseIsTrue[c] == 0) {
            // Клауза стала короче, увеличиваем её вес для свободных переменных
            double diff = 0.0;
            if (len < 127) diff = pows[len] - pows[len + 1];
            if (diff > 0.0) {
                for (int v_in_c : clauses[c].vars) {
                    int abs_v = abs(v_in_c);
                    if (usedVars[abs_v] == 0) {
                        if (v_in_c > 0) weightPos[abs_v] += diff;
                        else            weightNeg[abs_v] += diff;
                    }
                }
            }
        }

        if (!clauseIsTrue[c] && varsLeft[c] == 1)
            single.push(c);
        else if (!clauseIsTrue[c] && varsLeft[c] == 0)
            flag = 0;
    }
    lastVar.push(x);
    return flag;
}

void SATSolverJWH::backtrack() {
    while (!single.empty())
        single.pop();

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
    usedVars[curNum] = 0;

    for (auto c: *trueClauses) {
        clauseIsTrue[c]--;
        varsLeft[c]++;
        if (clauseIsTrue[c] == 0) {
            numZeroClauses++;
            // Клауза снова стала неудовлетворенной, возвращаем её вес
            int len = varsLeft[c];
            double w = (len < 128) ? pows[len] : 0.0;
            for (int v_in_c : clauses[c].vars) {
                int abs_v = abs(v_in_c);
                if (usedVars[abs_v] == 0) {
                    if (v_in_c > 0) weightPos[abs_v] += w;
                    else            weightNeg[abs_v] += w;
                }
            }
        }
    }

    for (auto c: *otherClauses) {
        int len = varsLeft[c];
        varsLeft[c]++;

        if (clauseIsTrue[c] == 0) {
            // Клауза стала длиннее, уменьшаем её вес
            double diff = 0.0;
            if (len < 127) diff = pows[len] - pows[len + 1];
            if (diff > 0.0) {
                for (int v_in_c : clauses[c].vars) {
                    int abs_v = abs(v_in_c);
                    if (usedVars[abs_v] == 0) {
                        if (v_in_c > 0) weightPos[abs_v] -= diff;
                        else            weightNeg[abs_v] -= diff;
                    }
                }
            }
        }
    }
    lastVar.pop();
}

Var SATSolverJWH::getNextVar() {
    Var needReturn{0, 0};
    while (!single.empty()) {
        int clause = single.top();
        single.pop();
        if (!clauseIsTrue[clause] && varsLeft[clause] == 1) {
            needReturn.var = findNotUsedVar(clauses[clause]);
            if (needReturn.var == 0)
                continue;
            needReturn.canChange = 0;
            return needReturn;
        }
    }

    double maxWeight = -1.0;
    int bestVar = -1;

    for (int i = 1; i <= numVars; ++i) {
        if (usedVars[i] == 0) {
            double currentWeight = weightPos[i] + weightNeg[i];
            if (currentWeight > maxWeight) {
                maxWeight = currentWeight;
                bestVar = i;
            }
        }
    }

    if (bestVar != -1) {
        needReturn.var = (weightPos[bestVar] >= weightNeg[bestVar]) ? bestVar : -bestVar;
        needReturn.canChange = 1;
        return needReturn;
    }

    needReturn.var = 0;
    return needReturn;
}
