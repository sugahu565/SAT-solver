#pragma once

#include "structures.hpp"
#include<vector>
#include<string>

class SATSolver { //структура для решения одной задачи
    private:
        int numVars;
        int numClauses;

        std::vector<short int> usedVars; // -1, 0, 1 - задействована ли переменная
        std::vector<Clause> clauses; // все клозы
        std::vector<VarInfo> vars; // инфа про каждую переменную - где лежит в негативном и позитивном виде

        std::vector<bool> clauseIsTrue; // какие клозы уже удовлетворены
        std::vector<int> varsLeft; // сколько переменных осталось в каждой клозе

        int solutionExist;
        
    public:
        SATSolver();
        bool DIMACS(std::string& nameFile);
        bool solve();
        Solution getSolution();
};

