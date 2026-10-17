#include "UndoAction.h"

UndoAction::UndoAction() 
    : type(ActionType::UNKNOWN), expenseId(-1), hasBefore(false), hasAfter(false) {}

UndoAction::UndoAction(ActionType t, long long id) 
    : type(t), expenseId(id), hasBefore(false), hasAfter(false) {}

UndoAction::UndoAction(ActionType t, long long id, const Expense& before, const Expense& after) 
    : type(t), expenseId(id), beforeSnapshot(before), hasBefore(true), afterSnapshot(after), hasAfter(true) {}

ActionType UndoAction::getType() const {
    return type;
}

long long UndoAction::getExpenseId() const {
    return expenseId;
}

bool UndoAction::getBeforeSnapshot(Expense& outExpense) const {
    if (!hasBefore) return false;
    outExpense = beforeSnapshot;
    return true;
}

void UndoAction::setBeforeSnapshot(const Expense& expense) {
    beforeSnapshot = expense;
    hasBefore = true;
}

bool UndoAction::getAfterSnapshot(Expense& outExpense) const {
    if (!hasAfter) return false;
    outExpense = afterSnapshot;
    return true;
}

void UndoAction::setAfterSnapshot(const Expense& expense) {
    afterSnapshot = expense;
    hasAfter = true;
}
