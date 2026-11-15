# PERSON 5 API DOCUMENTATION (100% COMPLETE MODULE)

## Module Overview
Person 5 is responsible for:
1. **SavingsAdvisor**: Financial calculations (monthly totals, run-rate forecast, potential savings, budget comparison).
2. **UndoAction**: Data representation of reversible actions (ADD, EDIT, DELETE) with snapshots.
3. **UndoStack**: Custom fixed-capacity stack (MAX_CAPACITY = 100) using static arrays (No STL containers/algorithms).

---

## 1. SavingsAdvisor Class
### Methods:
- `long long calculateMonthlyTotal(const ExpenseManager& manager, int month, int year) const`:
  Returns total expenses for given month/year in paise.
- `long long calculateMonthlyForecast(const ExpenseManager& manager, int month, int year) const`:
  Calculates simple run-rate projection based on current day of the month.
- `long long calculatePotentialSavings(const ExpenseManager& manager, int month, int year, int reductionPercent) const`:
  Returns savings amount based on `reductionPercent` (0 to 100%).
- `bool compareBudget(const ExpenseManager& manager, int month, int year, long long budgetPaise, long long& differencePaise) const`:
  Compares total vs budget. Returns `true` if within budget, `false` if over budget. `differencePaise` stores `budget - total`.

---

## 2. UndoAction Class
### Methods:
- `ActionType getType() const`: Returns ADD, EDIT, or DELETE.
- `long long getExpenseId() const`: Returns transaction ID.
- `bool getBeforeSnapshot(Expense& outExpense) const`: Retrieves pre-operation state.
- `bool getAfterSnapshot(Expense& outExpense) const`: Retrieves post-operation state.

---

## 3. UndoStack Class
### Methods:
- `bool push(const UndoAction& action)`: Push onto stack. Returns false if full.
- `bool pop(UndoAction& action)`: Pop from stack. Returns false if empty.
- `bool peek(UndoAction& action) const`: Reads top action without removing.
- `bool isEmpty() const`, `bool isFull() const`, `void clear()`, `int getSize() const`.

---

## 4. Person 6 Integration Protocol
When reversing actions popped from `UndoStack`:
- **ADD**: Call `ExpenseManager::deleteExpense(action.getExpenseId())`.
- **DELETE**: Call `ExpenseManager::addLoadedExpense(beforeSnapshot)` to restore original ID and timestamp.
- **EDIT**: Update existing expense back to `beforeSnapshot`.
