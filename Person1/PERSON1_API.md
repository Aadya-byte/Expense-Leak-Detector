# Person 1 API — Expense Model and ExpenseManager

## Files and responsibility
- `Expense.h/.cpp`: fixed-size expense value model and validation.
- `ExpenseNode.h/.cpp`: node holding one `Expense` by value and a link to the next node.
- `ExpenseManager.h/.cpp`: owns and manages all nodes in a singly linked list.
- `person1_test.cpp`: standalone tests. It contains a test `main()` only; do not compile it into the final app alongside the team's `main.cpp`.

## Build and test
```sh
g++ -std=c++11 -Wall -Wextra -pedantic Expense.cpp ExpenseNode.cpp ExpenseManager.cpp person1_test.cpp -o person1_test
./person1_test
```
Expected output:
```text
All Person 1 tests passed.
```

## Validation rules
- Dates must use `DD-MM-YYYY` and be valid calendar dates, including leap years. The date is stored and returned in this same format.
- `amountPaise` must be non-negative. Store currency as integer paise to avoid floating-point rounding.
- Merchant/category must be non-empty and fit their fixed-size arrays.
- Accepted `expenseType` values (exact, case-sensitive): `ONE_TIME` and `RECURRING`. Keep classifications such as `Essential`, `Discretionary`, `Subscription`, or `Impulse` in the existing `category` field, not `expenseType`.
- Accepted `frequency` values (exact, case-sensitive): `NONE`, `DAILY`, `WEEKLY`, and `MONTHLY`. `ONE_TIME` requires `NONE`; `RECURRING` requires `DAILY`, `WEEKLY`, or `MONTHLY`. All other values and mismatched type/frequency combinations are invalid.
- Maximum string lengths: date 10 chars; merchant 50; category 30; expense type 15; frequency 15. The null terminator also needs space.
- IDs must be positive; there is no public ID setter.

## Contract changes in this revision
- Date format is `DD-MM-YYYY`; validation checks both the format and the real calendar date, including leap-year rules.
- `expenseType` is restricted to `ONE_TIME` or `RECURRING`; descriptive categories remain in `category`.
- `frequency` is restricted to `NONE`, `DAILY`, `WEEKLY`, or `MONTHLY`, with `ONE_TIME`/`NONE` and `RECURRING`/(`DAILY`|`WEEKLY`|`MONTHLY`) combinations only.

## Ownership and lifetime
- `ExpenseManager` owns every node linked from `head`; it deletes nodes in `deleteExpense`, `clearExpenses`, and its destructor.
- `ExpenseNode` owns only its `Expense` value and does not own the manager or the rest of the list.
- `findExpenseById` returns a non-owning pointer. Never delete it. It becomes invalid when that record is edited/deleted, after `clearExpenses`, or when the manager is destroyed.
- `getHead()` is a non-owning pointer to a const node.
- `copyExpensePointers` copies non-owning pointers; it does not transfer ownership. Pointers are invalidated by edit/delete/clear/destruction as applicable.
- Copy construction/assignment of `ExpenseManager` are disabled to prevent shallow copies and double deletion.
- `clearExpenses()` resets the list/count but intentionally does not reset `nextExpenseId`, preventing ID reuse within a manager's lifetime.

## Function contracts
- `addExpense`: validates fields, assigns the next ID, and writes it to `assignedId` on success. Returns false and sets `assignedId` to zero on failure.
- `addLoadedExpense`: preserves the provided ID, rejects invalid/duplicate records, and advances the next ID past the largest loaded ID. If `INT_MAX` is loaded, new ID allocation is exhausted.
- `editExpense`: edits fields only for the requested existing record and retains the requested ID even if `updatedExpense` has a different ID.
- `deleteExpense`: removes/deletes one matching node; false if not found.
- `clearExpenses`: deletes all nodes and sets count to zero.
- `findExpenseById`: returns a pointer to the record or null if absent.
- `copyExpensePointers`: returns copied pointer count. Returns `-1` for null output, negative capacity, or capacity smaller than record count; on failure copies nothing.
- `displayExpenses`: prints each record, or a message when empty.

## Complexity
Let `n` be the number of records.
- `addExpense` / `addLoadedExpense`: `O(n)` due to duplicate-ID checking; insertion at head is `O(1)`.
- `findExpenseById`: `O(n)`.
- `editExpense`: `O(n)` to find the node.
- `deleteExpense`: `O(n)`.
- `clearExpenses` / destructor: `O(n)`.
- `copyExpensePointers`: `O(n)`.
- `getCount`, `getNextExpenseId`, `getHead`: `O(1)`.

## Integration notes
Compile `Expense.cpp`, `ExpenseNode.cpp`, and `ExpenseManager.cpp` with the other team members' `.cpp` files. Do not compile `person1_test.cpp` into the final executable when another `main.cpp` is present. No STL containers or algorithms are used.
