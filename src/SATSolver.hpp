#pragma once

#include "structures.hpp"
#include <cmath>
#include <stack>
#include <string>
#include <vector>

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
    std::stack<int> single; // индексы клауз, которые могли стать единичными
    std::stack<Var> lastVar; // стек присвоенных переменных

    int numZeroClauses; // сколько клауз ещё удов (для быстрого ответа)
    int solutionExist;

    int findNotUsedVar(const Clause& curr);

    // Базовые реализации (невиртуальные для инлайнинга)
    bool addVar(Var x);
    void backtrack();

public:
    SATSolver();
    virtual ~SATSolver() = default;
    bool dimacs(std::string& nameFile);
    [[nodiscard]] Solution getSolution() const;

    virtual bool solve() = 0; // Чисто виртуальный метод для запуска решателя
    [[nodiscard]] virtual const char* heuristicName() const = 0;
};

class SATSolverNaive : public SATSolver {
private:
    Var getNextVar();

public:
    bool solve() override;
    [[nodiscard]] const char* heuristicName() const override { return "naive"; }
};

class SATSolverJWH : public SATSolver {
private:
    std::vector<double> pows; // 2^(-i)
    std::vector<double> weightPos;
    std::vector<double> weightNeg;

    void initHeuristic();
    bool addVar(Var x); // Перекрывает базовый метод (Shadowing)
    void backtrack(); // Перекрывает базовый метод
    Var getNextVar();

public:
    SATSolverJWH();
    bool solve() override;
    [[nodiscard]] const char* heuristicName() const override { return "jwh"; }
};
