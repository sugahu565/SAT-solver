#pragma once

#include "structures.hpp"
#include<vector>
#include<string>
#include<stack>

class SATSolver { //структура для решения одной задачи
    private:
        int numVars;
        int numClauses;

        int numZeroClauses; // сколько клауз ещё удов (для быстрого ответа)

        std::vector<short int> usedVars; // -1, 0, 1 - задействована ли переменная
        std::vector<Clause> clauses; // все клозы
        std::vector<VarInfo> vars; // инфа про каждую переменную - где лежит в негативном и позитивном виде

        std::vector<int> clauseIsTrue; // какие клозы уже удовлетворены и скольким кол-вом переменных
        std::vector<int> varsLeft; // сколько переменных осталось в каждой клозе
        
        std::vector<int> single; // какие клозы остались с одним значением
        std::stack<Var> lastVal; // стек присвоенных переменных
        // первый int для переменной
        // второй int: 0 если нельзя поменять знак, 1 если можно

        int solutionExist;

        bool addVar(Var x); // присваиваю переменной значение (ОДНОЙ), внутри обновляю все вектора и тд и смотрю на противоречия
        void backtrack(); // удаляю значение переменной, меняю все вектора и тд
        Var findNextVar(); // находит след переменные для присвоения (внутри проверка на непустой список единичных, иначе тупо ищу)
    public:
        SATSolver();
        bool DIMACS(std::string& nameFile);
        bool solve();
        Solution getSolution();
};

