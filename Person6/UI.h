#ifndef UI_H
#define UI_H

#include "ExpenseManager.h"
#include "LeakAnalyzer.h"
#include "Search.h"
#include "Sort.h"
#include "ExpenseBST.h"
#include "SavingsAdvisor.h"
#include "UndoStack.h"

class FileManager; // Implemented by Person 2; keep its definition in its own module.

class UI {
private:
    ExpenseManager& manager;
    FileManager& fileManager;
    LeakAnalyzer& leakAnalyzer;
    Search& search;
    Sort& sort;
    ExpenseBST& expenseBST;
    SavingsAdvisor& savingsAdvisor;
    UndoStack& undoStack;

    bool readInt(const char* prompt, int& value) const;
    bool readPositiveAmount(const char* prompt, long long& value) const;
    bool readText(const char* prompt, char* buffer, int capacity) const;
    void addExpense();
    void viewExpenses() const;
    void editExpense();
    void deleteExpense();
    void searchExpense();
    void sortExpenses();
    void showLeakReport();
    void showForecast();
    void undoLast();
    void exportMonthlyReport();

public:
    UI(ExpenseManager& manager,
       FileManager& fileManager,
       LeakAnalyzer& leakAnalyzer,
       Search& search,
       Sort& sort,
       ExpenseBST& expenseBST,
       SavingsAdvisor& savingsAdvisor,
       UndoStack& undoStack);

    void run();
    void displayMainMenu() const;
};

#endif
