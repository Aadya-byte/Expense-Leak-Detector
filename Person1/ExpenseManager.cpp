#include "ExpenseManager.h"
#include <iostream>
#include <climits>

ExpenseManager::ExpenseManager()
    : head(0), recordCount(0), nextExpenseId(1) {}

ExpenseManager::~ExpenseManager() {
    releaseAllNodes();
}

bool ExpenseManager::containsId(int idValue) const {
    const ExpenseNode* current = head;
    while (current != 0) {
        if (current->getExpense().getId() == idValue) return true;
        current = current->getNext();
    }
    return false;
}

void ExpenseManager::releaseAllNodes() {
    ExpenseNode* current = head;
    while (current != 0) {
        ExpenseNode* next = current->getNext();
        delete current;
        current = next;
    }
    head = 0;
    recordCount = 0;
}

bool ExpenseManager::addExpense(const char* date, long long amountPaise,
                                const char* merchant, const char* category,
                                const char* expenseType, const char* frequency,
                                int& assignedId) {
    assignedId = 0;
    // INT_MAX is kept as the exhausted sentinel; avoid signed overflow.
    if (nextExpenseId <= 0 || nextExpenseId == INT_MAX) return false;

    Expense candidate(nextExpenseId, date, amountPaise, merchant, category,
                      expenseType, frequency);
    if (!candidate.isValid() || containsId(candidate.getId())) return false;

    ExpenseNode* node = new ExpenseNode(candidate, head);
    head = node;
    ++recordCount;
    assignedId = nextExpenseId;
    ++nextExpenseId;
    return true;
}

bool ExpenseManager::addLoadedExpense(const Expense& expenseValue) {
    if (!expenseValue.isValid() || containsId(expenseValue.getId())) return false;

    ExpenseNode* node = new ExpenseNode(expenseValue, head);
    head = node;
    ++recordCount;

    if (expenseValue.getId() >= nextExpenseId) {
        if (expenseValue.getId() == INT_MAX) nextExpenseId = INT_MAX;
        else nextExpenseId = expenseValue.getId() + 1;
    }
    return true;
}

bool ExpenseManager::editExpense(int idValue, const Expense& updatedExpense) {
    if (idValue <= 0 || !updatedExpense.isValid()) return false;

    ExpenseNode* current = head;
    ExpenseNode* previous = 0;
    while (current != 0) {
        if (current->getExpense().getId() == idValue) {
            // Copy editable fields but retain the original, stable ID.
            Expense replacement(idValue, updatedExpense.getDate(),
                                updatedExpense.getAmountPaise(),
                                updatedExpense.getMerchant(),
                                updatedExpense.getCategory(),
                                updatedExpense.getExpenseType(),
                                updatedExpense.getFrequency());
            if (!replacement.isValid()) return false;

            ExpenseNode* replacementNode = new ExpenseNode(replacement, current->getNext());
            if (previous == 0) head = replacementNode;
            else previous->setNext(replacementNode);
            delete current;
            return true;
        }
        previous = current;
        current = current->getNext();
    }
    return false;
}

bool ExpenseManager::deleteExpense(int idValue) {
    ExpenseNode* current = head;
    ExpenseNode* previous = 0;

    while (current != 0) {
        if (current->getExpense().getId() == idValue) {
            if (previous == 0) head = current->getNext();
            else previous->setNext(current->getNext());
            delete current;
            --recordCount;
            return true;
        }
        previous = current;
        current = current->getNext();
    }
    return false;
}

bool ExpenseManager::clearExpenses() {
    releaseAllNodes();
    // IDs are not reused during the manager's lifetime.
    return true;
}

Expense* ExpenseManager::findExpenseById(int idValue) {
    ExpenseNode* current = head;
    while (current != 0) {
        if (current->getExpense().getId() == idValue)
            return const_cast<Expense*>(&current->getExpense());
        current = current->getNext();
    }
    return 0;
}

const Expense* ExpenseManager::findExpenseById(int idValue) const {
    const ExpenseNode* current = head;
    while (current != 0) {
        if (current->getExpense().getId() == idValue)
            return &current->getExpense();
        current = current->getNext();
    }
    return 0;
}

int ExpenseManager::getCount() const { return recordCount; }
int ExpenseManager::getNextExpenseId() const { return nextExpenseId; }
const ExpenseNode* ExpenseManager::getHead() const { return head; }

int ExpenseManager::copyExpensePointers(Expense* output[], int capacity) const {
    if (output == 0 || capacity < 0 || capacity < recordCount) return -1;

    const ExpenseNode* current = head;
    int index = 0;
    while (current != 0) {
        output[index++] = const_cast<Expense*>(&current->getExpense());
        current = current->getNext();
    }
    return index;
}

void ExpenseManager::displayExpenses() const {
    if (head == 0) {
        std::cout << "No expenses found.\n";
        return;
    }
    const ExpenseNode* current = head;
    while (current != 0) {
        current->getExpense().display();
        current = current->getNext();
    }
}
