#pragma once

#include "structures.hpp"
#include<vector>
#include<string>
#include<stack>

class SATSolver { //структура для решения одной задачи
    private:
        int numVars;
        int numClauses;

        std::vector<short int> usedVars; // -1, 0, 1 - задействована ли переменная
        std::vector<Clause> clauses; // все клозы
        std::vector<VarInfo> vars; // инфа про каждую переменную - где лежит в негативном и позитивном виде

        std::vector<int> clauseIsTrue; // какие клозы уже удовлетворены и скольким кол-вом переменных
        std::vector<int> varsLeft; // сколько переменных осталось в каждой клозе
        
        std::vector<int> single; // какие клозы остались с одним значением
        std::stack<int> lastVal; // стек присвоенных переменных

        int solutionExist;

        bool addVal(int x); // пытаюсь присвоить переменной значение, внутри обновляю все вектора и тд и смотрю на противоречия
        void backtrack(int x); // удаляю значение переменной, меняю все вектора и тд
        int findNextVal(); // найти след переменную для присвоения
    public:
        SATSolver();
        bool DIMACS(std::string& nameFile);
        bool solve();
        Solution getSolution();
};

