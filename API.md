# Person 1 - Expense Data & Linked List (OOP core) - API of the 40% delivery

## Public interface delivered

- `Expense(int id, const char* date, long long amountPaise, const char* merchant, const char* category, const char* expenseType, const char* frequency)`
- `bool setDate/setAmountPaise/setMerchant/setCategory/setExpenseType/setFrequency(...)`
- `bool isValid() const;  void display() const;  getters: getId, getDate, getAmountPaise, getMerchant, getCategory, getExpenseType, getFrequency`
- `bool ExpenseManager::addExpense(date, amountPaise, merchant, category, expenseType, frequency, int& assignedId)`
- `bool ExpenseManager::addLoadedExpense(const Expense&)`
- `bool ExpenseManager::deleteExpense(int id)`
- `Expense* ExpenseManager::findExpenseById(int id)  (non-owning pointer)`
- `int getCount() const;  int getNextExpenseId() const;  const ExpenseNode* getHead() const`
- `int copyExpensePointers(Expense* output[], int capacity) const  (-1 on error)`
- `void displayExpenses() const`

## Still pending (Phase 3)

- editExpense() (edit an existing record, keep its ID)
- clearExpenses() (reset list)
- RecurringExpense : public Expense (inheritance) with frequency + projected monthly amount, virtual monthly-estimate method
- operator> (compare expenses by amount) and a friend display helper
- ExpenseTypes.h shared constants, extended edge-case test-suite, complexity write-up
