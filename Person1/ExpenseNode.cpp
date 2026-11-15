#include "ExpenseNode.h"

ExpenseNode::ExpenseNode(const Expense& expenseValue, ExpenseNode* nextValue)
    : expense(expenseValue), next(nextValue) {}

const Expense& ExpenseNode::getExpense() const { return expense; }
ExpenseNode* ExpenseNode::getNext() { return next; }
const ExpenseNode* ExpenseNode::getNext() const { return next; }
void ExpenseNode::setNext(ExpenseNode* nextValue) { next = nextValue; }
