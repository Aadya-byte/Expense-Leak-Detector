# Integration Checklist — Person 6

## Shared contracts
- [x] Person 1's `Expense` / `ExpenseManager` are the application's canonical model.
- [x] Person 2 persistence/parser/export code uses the canonical model API.
- [x] Person 3's analyzer traverses Person 1's by-value `ExpenseNode` list.
- [x] Person 4 search, sort, and amount-index modules are wired into the UI.
- [x] Person 5 savings and undo modules use the canonical manager API.
- [x] Alternate Person 2/3 headers and test adapters are excluded from the app build.
- [x] Persistence uses the existing pipe-delimited text snapshot; no database,
  JSON, or SQL was introduced.
- [x] Expense changes save immediately to `expense.txt` beside the executable.
- [x] Monthly reports are exported beside `expense.txt`.

## Verification completed
- [x] Full application build with `g++ -std=c++11 -Wall -Wextra -pedantic`.
- [x] Integrated test executable: 46 checks passed, 0 failed.
- [x] Person 1 regression tests passed.
- [x] End-to-end app run: add, analyze, export, save, restart/load, and view.
- [x] Search details are presented in an organized block.
- [x] Search results and all sort modes use the same aligned table.
- [x] Sorted results group merchant rows under their category headings.
- [x] Exported monthly report includes month-filtered leak analysis and forecast.
- [x] Save/load preserves expense IDs and values.
- [x] Parser rejects overflowing IDs and malformed delimiter characters.
- [x] Integer-boundary savings calculation does not overflow.

## Remaining manual coverage
- [ ] Exercise edit/delete and undo ADD, EDIT, and DELETE through interactive input.
- [ ] Exercise invalid menu input and invalid expense fields interactively.
- [ ] Review the generated report contents for a representative dataset.

## Build note
Compile `Person6\main.cpp` for the application, or `Person6\IntegrationTest.cpp`
for tests; never compile both into one executable. The integration compile
command and storage/report locations are documented in `README.md`.
