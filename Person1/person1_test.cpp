#include "ExpenseManager.h"
#include <iostream>

int main() {
    ExpenseManager manager;
    int id1 = 0, id2 = 0;

    if (!manager.addExpense("16-10-2026", 12550, "Cafe Aroma", "Food",
                            "ONE_TIME", "NONE", id1)) return 1;
    if (!manager.addExpense("15-10-2026", 49900, "StreamBox", "Subscription",
                            "RECURRING", "MONTHLY", id2)) return 2;
    if (manager.getCount() != 2 || id1 == id2) return 3;
    if (manager.findExpenseById(id1) == 0) return 4;

    Expense duplicate(id1, "14-10-2026", 100, "Shop", "Other", "ONE_TIME", "NONE");
    if (manager.addLoadedExpense(duplicate)) return 5;

    Expense loaded(25, "14-10-2026", 1000, "Bus", "Travel", "RECURRING", "DAILY");
    if (!manager.addLoadedExpense(loaded) || manager.getNextExpenseId() != 26) return 6;

    Expense update(999, "13-10-2026", 2000, "Cafe Aroma", "Food", "ONE_TIME", "NONE");
    if (!manager.editExpense(id1, update)) return 7;
    if (manager.findExpenseById(id1)->getId() != id1) return 8;
    if (manager.findExpenseById(id1)->getAmountPaise() != 2000) return 9;

    Expense* pointers[3];
    if (manager.copyExpensePointers(pointers, 3) != 3) return 10;
    if (manager.copyExpensePointers(pointers, 2) != -1) return 11;
    if (manager.copyExpensePointers(0, 3) != -1) return 12;

    if (!manager.deleteExpense(id2) || manager.deleteExpense(id2)) return 13;
    if (!manager.clearExpenses() || manager.getCount() != 0) return 14;

    // Actual calendar validation and DD-MM-YYYY format.
    Expense leap(4, "29-02-2024", 100, "Shop", "Other", "ONE_TIME", "NONE");
    if (!leap.isValid()) return 15;
    Expense invalidDay(4, "30-02-2024", 100, "Shop", "Other", "ONE_TIME", "NONE");
    if (invalidDay.isValid()) return 16;
    Expense invalidLeap(4, "29-02-2023", 100, "Shop", "Other", "ONE_TIME", "NONE");
    if (invalidLeap.isValid()) return 17;
    Expense oldFormat(4, "2024-02-29", 100, "Shop", "Other", "ONE_TIME", "NONE");
    if (oldFormat.isValid()) return 18;
    Expense invalidMonth(4, "10-13-2024", 100, "Shop", "Other", "ONE_TIME", "NONE");
    if (invalidMonth.isValid()) return 19;

    // Only ONE_TIME / RECURRING and exact frequency values are accepted.
    Expense badType(5, "01-01-2026", 100, "Shop", "Essential", "Essential", "NONE");
    if (badType.isValid()) return 20;
    Expense badFrequency(5, "01-01-2026", 100, "Shop", "Subscription", "RECURRING", "YEARLY");
    if (badFrequency.isValid()) return 21;
    Expense oneTimeWrongFreq(5, "01-01-2026", 100, "Shop", "Food", "ONE_TIME", "MONTHLY");
    if (oneTimeWrongFreq.isValid()) return 22;
    Expense recurringWrongFreq(5, "01-01-2026", 100, "Shop", "Food", "RECURRING", "NONE");
    if (recurringWrongFreq.isValid()) return 23;
    Expense lowerCase(5, "01-01-2026", 100, "Shop", "Food", "recurring", "weekly");
    if (lowerCase.isValid()) return 24;

    // Manager must reject invalid combinations too.
    int rejectedId = -1;
    if (manager.addExpense("01-01-2026", 100, "Shop", "Food", "ONE_TIME", "DAILY", rejectedId)) return 25;
    if (rejectedId != 0) return 26;
    if (manager.addExpense("01-01-2026", 100, "Shop", "Food", "RECURRING", "NONE", rejectedId)) return 27;

    std::cout << "All Person 1 tests passed (existing and new validation cases).\n";
    return 0;
}
