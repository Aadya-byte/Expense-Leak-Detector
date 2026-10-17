# Person 1 - Expense Data & Linked List (OOP core)

**Phase 2 delivery - about 40%% of the planned work.**

**Share of the supplied source delivered:** 91% of the supplied Person 1 source (foundation module: every other module depends on it).

## What is done (40%)

- Expense class: fixed-size char arrays (no STL), constructors, getters, validating setters, isValid(), display()
- Validation: real calendar dates in DD-MM-YYYY (leap years), non-negative paise, non-empty bounded text, ONE_TIME/RECURRING with consistent NONE/DAILY/WEEKLY/MONTHLY frequency
- ExpenseNode: singly-linked-list node (stores one Expense by value + next pointer)
- ExpenseManager: addExpense (unique auto ID), addLoadedExpense (keeps file ID, rejects duplicates), deleteExpense (head / middle / last / missing ID), findExpenseById, getCount, getNextExpenseId, getHead, copyExpensePointers (for search/sort), displayExpenses
- Destructor releases every node; copy constructor / assignment disabled to prevent double deletion
- person1_test.cpp: unit tests for validation, add/find/delete, loaded IDs, pointer copy, empty list

## What is NOT done yet (remaining 60%)

- editExpense() (edit an existing record, keep its ID)
- clearExpenses() (reset list)
- RecurringExpense : public Expense (inheritance) with frequency + projected monthly amount, virtual monthly-estimate method
- operator> (compare expenses by amount) and a friend display helper
- ExpenseTypes.h shared constants, extended edge-case test-suite, complexity write-up

## Folder layout

- `src/` - this member's own files: Expense.h, Expense.cpp, ExpenseNode.h, ExpenseNode.cpp, ExpenseManager.h, ExpenseManager.cpp
- `tests/` - person1_test.cpp
- `API.md` - public functions delivered and pending

- `Makefile` - `make test` compiles and runs this module's test

## Build and test

```sh
make test
```

Expected output: `All Person 1 (40% scope) tests passed.`

## Rules followed

- C++11, no STL containers or algorithms (`vector`, `list`, `map`, `stack`, `sort()` ... are not used).
- Money is stored as integer paise (15000 = Rs. 150.00); dates are DD-MM-YYYY.
- One shared `Expense` / `ExpenseManager` definition (Person 1's); no module keeps a second master list of expenses.
