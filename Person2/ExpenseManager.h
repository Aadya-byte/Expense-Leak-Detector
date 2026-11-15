#ifndef EXPENSE_MANAGER_H
#define EXPENSE_MANAGER_H

/*
 * ============================================================================
 * TEAM CONTRACT SPECIFICATION - EXPENSE MANAGER INTERFACE
 * Owned by: Person 1 (Core Model & Singly Linked List)
 * Referenced by: Person 2 (Persistence & Export Module)
 *
 * NOTE: This header defines the exact contract interface for ExpenseManager
 * agreed upon in Section 5 and Section 7 of the team contract.
 * ============================================================================
 */

#include "Expense.h"

class ExpenseManager {
private:
    ExpenseNode* m_head;
    int m_count;

public:
    ExpenseManager();
    virtual ~ExpenseManager();

    // Traversal and inspection interface
    ExpenseNode* getHead() const;
    int getCount() const;

    // Insertion that preserves file ID without auto-generating a new one
    // Used by FileManager during load operations
    bool addLoadedExpense(const Expense& expense);
    bool addLoadedExpense(Expense* expense);

    // Normal insertion (allocates new ID if needed)
    bool addExpense(const Expense& expense);

    // Query methods
    bool containsId(int id) const;
    Expense* findExpenseById(int id) const;

    // Clear all records (frees all nodes and their expenses)
    void clear();
};

#endif // EXPENSE_MANAGER_H
