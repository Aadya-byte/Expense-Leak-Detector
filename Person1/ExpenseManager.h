#ifndef EXPENSE_MANAGER_H
#define EXPENSE_MANAGER_H

#include "ExpenseNode.h"

class ExpenseManager {
private:
    ExpenseNode* head;
    int recordCount;
    int nextExpenseId;

    bool containsId(int id) const;
    void releaseAllNodes();

    // Owning linked lists must not be shallow-copied.
    ExpenseManager(const ExpenseManager&);
    ExpenseManager& operator=(const ExpenseManager&);

public:
    ExpenseManager();
    ~ExpenseManager();

    bool addExpense(const char* date, long long amountPaise,
                    const char* merchant, const char* category,
                    const char* expenseType, const char* frequency,
                    int& assignedId);

    bool addLoadedExpense(const Expense& expense);
    bool editExpense(int id, const Expense& updatedExpense);
    bool deleteExpense(int id);
    bool clearExpenses();

    Expense* findExpenseById(int id);
    const Expense* findExpenseById(int id) const;

    int getCount() const;
    int getNextExpenseId() const;
    const ExpenseNode* getHead() const;

    int copyExpensePointers(Expense* output[], int capacity) const;
    void displayExpenses() const;
};

#endif
