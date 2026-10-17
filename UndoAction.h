#ifndef UNDO_ACTION_H
#define UNDO_ACTION_H

#include "Expense.h"

enum class ActionType {
    ADD,
    EDIT,
    DELETE,
    UNKNOWN
};

class UndoAction {
private:
    ActionType type;
    long long expenseId;
    Expense beforeSnapshot;
    bool hasBefore;
    Expense afterSnapshot;
    bool hasAfter;

public:
    UndoAction();
    UndoAction(ActionType t, long long id);
    UndoAction(ActionType t, long long id, const Expense& before, const Expense& after);

    ActionType getType() const;
    long long getExpenseId() const;
    
    bool getBeforeSnapshot(Expense& outExpense) const;
    void setBeforeSnapshot(const Expense& expense);

    bool getAfterSnapshot(Expense& outExpense) const;
    void setAfterSnapshot(const Expense& expense);
};

#endif // UNDO_ACTION_H
