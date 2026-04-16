#pragma once
#include<vector>

struct Solution { //для вывода ответа
    bool solutionExist = 0;
    std::vector<int> variables;
};

struct Clause {
    int numOfVars = 0;
    std::vector<int> vars;
};

struct VarInfo {
    int numPosOccur = 0;
    int numNegOccur = 0;

    std::vector<int> posOccur;
    std::vector<int> negOccur;
};

struct Var {
    int var;
    int canChange;
};

