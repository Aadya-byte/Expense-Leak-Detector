// Person 1 - Phase 2 unit test (Expense model + linked-list ExpenseManager, 40% scope)
#include "ExpenseManager.h"
#include <iostream>

#define CHECK(c) do { if (!(c)) { std::cout << "FAIL line " << __LINE__ << ": " #c "\n"; return 1; } } while (0)

int main() {
    ExpenseManager m;
    int a = 0, b = 0, bad = -1;
    CHECK(m.getCount() == 0);
    CHECK(m.addExpense("16-10-2026", 12550, "Cafe Aroma", "Food", "ONE_TIME", "NONE", a));
    CHECK(m.addExpense("15-10-2026", 49900, "StreamBox", "Subscription", "RECURRING", "MONTHLY", b));
    CHECK(m.getCount() == 2 && a != b && a > 0 && b > 0);
    CHECK(m.findExpenseById(a) != 0 && m.findExpenseById(999) == 0);

    // validation: calendar dates, formats, type/frequency rules
    Expense leap(4, "29-02-2024", 100, "Shop", "Other", "ONE_TIME", "NONE");  CHECK(leap.isValid());
    Expense d1(4, "30-02-2024", 100, "Shop", "Other", "ONE_TIME", "NONE");    CHECK(!d1.isValid());
    Expense d2(4, "29-02-2023", 100, "Shop", "Other", "ONE_TIME", "NONE");    CHECK(!d2.isValid());
    Expense d3(4, "2024-02-29", 100, "Shop", "Other", "ONE_TIME", "NONE");    CHECK(!d3.isValid());
    Expense t1(5, "01-01-2026", 100, "Shop", "Food", "ONE_TIME", "MONTHLY"); CHECK(!t1.isValid());
    Expense t2(5, "01-01-2026", 100, "Shop", "Food", "RECURRING", "NONE");   CHECK(!t2.isValid());
    CHECK(!m.addExpense("01-01-2026", 100, "Shop", "Food", "ONE_TIME", "DAILY", bad) && bad == 0);

    // loaded record keeps its ID, duplicates rejected, next ID moves past it
    Expense loaded(25, "14-10-2026", 1000, "Bus", "Travel", "RECURRING", "DAILY");
    CHECK(m.addLoadedExpense(loaded) && m.getNextExpenseId() == 26);
    CHECK(!m.addLoadedExpense(loaded));

    // pointer copy (used by search / sort)
    Expense* p[3];
    CHECK(m.copyExpensePointers(p, 3) == 3);
    CHECK(m.copyExpensePointers(p, 2) == -1 && m.copyExpensePointers(0, 3) == -1);

    // delete: invalid id, middle/head/last node
    CHECK(!m.deleteExpense(777));
    CHECK(m.deleteExpense(b) && !m.deleteExpense(b) && m.getCount() == 2);
    CHECK(m.deleteExpense(25) && m.deleteExpense(a) && m.getCount() == 0);
    m.displayExpenses(); // empty-list message, must not crash
    std::cout << "All Person 1 (40% scope) tests passed.\n";
    return 0;
}
