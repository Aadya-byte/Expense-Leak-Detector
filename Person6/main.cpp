#include <iostream>
#include "ExpenseManager.h"
#include "FileManager.h"
#include "LeakAnalyzer.h"
#include "Search.h"
#include "Sort.h"
#include "ExpenseBST.h"
#include "SavingsAdvisor.h"
#include "UndoStack.h"
#include "UI.h"
#include <cstring>

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#include <windows.h>
#endif

int main() {
    ExpenseManager manager;
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
    char expensePath[MAX_PATH];
    DWORD pathLength = GetModuleFileNameA(NULL, expensePath, MAX_PATH);
    if (pathLength == 0 || pathLength >= MAX_PATH) {
        std::cerr << "Could not determine the application folder for expense.txt.\n";
        return 1;
    }
    char* pathSeparator = std::strrchr(expensePath, '\\');
    char* forwardSlash = std::strrchr(expensePath, '/');
    if (forwardSlash != NULL && (pathSeparator == NULL || forwardSlash > pathSeparator))
        pathSeparator = forwardSlash;
    if (pathSeparator == NULL) {
        std::cerr << "Could not determine the application folder for expense.txt.\n";
        return 1;
    }
    std::strcpy(pathSeparator + 1, "expense.txt");
    FileManager fileManager(expensePath);
#else
    FileManager fileManager;
#endif
    if (!fileManager.loadExpenses(manager)) {
        std::cerr << "Could not load expenses: " << fileManager.getLastError() << "\n";
        return 1;
    }

    LeakAnalyzer leakAnalyzer;
    Search search;
    Sort sort;
    ExpenseBST expenseBST;
    SavingsAdvisor savingsAdvisor;
    UndoStack undoStack;

    UI ui(manager, fileManager, leakAnalyzer, search, sort,
          expenseBST, savingsAdvisor, undoStack);
    ui.run();

    if (!fileManager.saveExpenses(manager)) {
        std::cerr << "Could not save expenses: " << fileManager.getLastError() << "\n";
        return 1;
    }
    std::cout << "Expenses saved. Exiting Expense Leak Detector.\n";
    return 0;
}
