#include "Search.h"

Search::Search() {}

Search::~Search() {}

Expense* Search::linearSearchById(Expense* expenses[], int count, int id) const {
    if (!expenses || count <= 0) {
        return nullptr;
    }

    for (int i = 0; i < count; ++i) {
        if (expenses[i] && expenses[i]->getId() == id) {
            return expenses[i];
        }
    }

    return nullptr;
}

Expense* Search::binarySearchById(Expense* sortedExpenses[], int count, int id) const {
    if (!sortedExpenses || count <= 0) {
        return nullptr;
    }

    int low = 0;
    int high = count - 1;

    while (low <= high) {
        int mid = low + (high - low) / 2;

        if (!sortedExpenses[mid]) {
            // Defensive safeguard against unexpected null pointers
            // If mid is null, perform a linear probe within the current range
            bool foundNonNull = false;
            for (int k = low; k <= high; ++k) {
                if (sortedExpenses[k]) {
                    int kId = sortedExpenses[k]->getId();
                    if (kId == id) return sortedExpenses[k];
                    if (kId > id) { high = k - 1; foundNonNull = true; break; }
                }
            }
            if (!foundNonNull) return nullptr;
            continue;
        }

        int midId = sortedExpenses[mid]->getId();

        if (midId == id) {
            return sortedExpenses[mid];
        } else if (midId < id) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }

    return nullptr;
}
