# Person 5 Module - Expense Management System

## Files List
- `UndoAction.h` / `UndoAction.cpp`: Reversible action model & snapshots.
- `UndoStack.h` / `UndoStack.cpp`: Array-based fixed stack implementation.
- `SavingsAdvisor.h` / `SavingsAdvisor.cpp`: Financial calculations & forecasting.
- `person5_test.cpp`: Automated unit tests.
- `PERSON5_API.md`: Full API integration document for Person 6.

## Compilation Instructions
```bash
g++ -std=c++11 UndoAction.cpp UndoStack.cpp SavingsAdvisor.cpp person5_test.cpp -o person5_test
./person5_test
```
