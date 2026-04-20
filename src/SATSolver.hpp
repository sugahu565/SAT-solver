#pragma once

#include "structures.hpp"
#include<vector>
#include<string>
#include<stack>

class SATSolver { //структура для решения одной задачи
    private:
        // Объекты, которые не меняются внутри solve
        int numVars;
        int numClauses;

        std::vector<Clause> clauses; // все клозы
        std::vector<VarInfo> varsInfo; // инфа про каждую переменную - где лежит в негативном и позитивном виде

        // Объекты, которые меняются внутри solve

        std::vector<int> clauseIsTrue; // какие клозы уже удовлетворены и скольким кол-вом переменных
        std::vector<int> varsLeft; // сколько переменных осталось в каждой клозе
        
        std::vector<short int> usedVars; // -1, 0, 1 - задействована ли переменная
        std::stack<int> single; // какие клозы остались с одним значением
        std::stack<Var> lastVar; // стек присвоенных переменных
        // первый int для переменной
        // второй int: 0 если нельзя поменять знак, 1 если можно
        
        int numZeroClauses; // сколько клауз ещё удов (для быстрого ответа)
        int solutionExist;

        bool addVar(Var x); // присваиваю переменной значение (ОДНОЙ), внутри обновляю все вектора и тд и смотрю на противоречия
        void backtrack(); // удаляю значение переменной, меняю все вектора и тд
        Var getNextVar(); // находит след переменные для присвоения (внутри проверка на непустой список единичных, иначе тупо ищу)
        int findNotUsedVar(const Clause& curr);
    public:
        SATSolver();
        bool DIMACS(std::string& nameFile);
        bool solve();
        Solution getSolution();
};

