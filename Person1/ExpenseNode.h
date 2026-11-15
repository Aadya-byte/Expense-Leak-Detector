#ifndef EXPENSE_NODE_H
#define EXPENSE_NODE_H

#include "Expense.h"

class ExpenseNode {
private:
    Expense expense;
    ExpenseNode* next;

public:
    ExpenseNode(const Expense& expense, ExpenseNode* next = 0);
    const Expense& getExpense() const;
    ExpenseNode* getNext();
    const ExpenseNode* getNext() const;
    void setNext(ExpenseNode* next);
};

#endif
