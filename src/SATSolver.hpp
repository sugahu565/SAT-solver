#pragma once

#include "structures.hpp"
#include <vector>
#include <string>
#include <stack>
#include <cmath>

class SATSolver { // базовый класс для решения одной задачи
    protected:
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
        
        int numZeroClauses; // сколько клауз ещё удов (для быстрого ответа)
        int solutionExist;

        int findNotUsedVar(const Clause& curr);

    public:
        SATSolver();
        virtual ~SATSolver() = default;
        bool dimacs(std::string& nameFile);
        bool solve();
        Solution getSolution();

        virtual void initHeuristic() {}
        virtual bool addVar(Var x); // присваиваю переменной значение
        virtual void backtrack(); // удаляю значение переменной
        virtual Var getNextVar() = 0; // находит след переменные для присвоения
};

class SATSolverNaive : public SATSolver {
    public:
        Var getNextVar() override;
};

class SATSolverJWH : public SATSolver {
    private:
        std::vector<double> pows; // 2^(-i)
        std::vector<double> weightPos;
        std::vector<double> weightNeg;

    public:
        SATSolverJWH();
        void initHeuristic() override;
        bool addVar(Var x) override;
        void backtrack() override;
        Var getNextVar() override;
};
