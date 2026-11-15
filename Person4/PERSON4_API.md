# Person 4 Module Documentation: Search, Sort and ExpenseBST

## 1. Overview
The **Person 4 Module** provides core searching, sorting, and binary search tree indexing algorithms for the Expense Leak Detector application. It is engineered to operate on non-owning pointers extracted from Person 1's central `ExpenseManager` singly linked list, maintaining strict non-destructive semantics and conforming to a strict **no-STL** policy (no `std::vector`, `std::map`, `std::list`, `std::string`, `std::sort`, or `<algorithm>`).

All monetary amounts are represented as 64-bit integer paise (`long long amountPaise`, where ₹1.00 = 100 paise) to prevent floating-point rounding errors.

---

## 2. File Manifest

| File | Purpose |
| :--- | :--- |
| `Search.h` / `Search.cpp` | Linear search on unsorted arrays; binary search on arrays sorted by ID ascending. |
| `Sort.h` / `Sort.cpp` | Manual Quicksort implementations for sorting by ID, amount, and date. |
| `BSTNode.h` / `BSTNode.cpp` | Index-only node holding `(expenseId, amountPaise, left, right)` without owning Expense data. |
| `ExpenseBST.h` / `ExpenseBST.cpp` | Amount-indexed binary search tree with composite tie-breaker key `(amountPaise, expenseId)`. |
| `person4_test.cpp` | Standalone validation driver testing empty arrays, single elements, duplicates, unsorted/sorted data, and integration. |
| `PERSON4_API.md` | Comprehensive architectural, API, complexity, and integration documentation. |

---

## 3. Class API Reference

### 3.1 `Search` Class
Header: `Search.h`  
Namespace: Global

```cpp
class Search {
public:
    Search();
    ~Search();

    Expense* linearSearchById(Expense* expenses[], int count, int id) const;
    Expense* binarySearchById(Expense* sortedExpenses[], int count, int id) const;
};
```

#### Functions

#### `Expense* linearSearchById(Expense* expenses[], int count, int id) const`
- **Description**: Sequentially traverses the array of non-owning Expense pointers to find the first record matching `id`.
- **Preconditions**: None. Operates correctly on unsorted, partially sorted, or already-sorted arrays.
- **Parameters**:
  - `expenses`: Array of non-owning pointers to `Expense` objects. Can be `nullptr`.
  - `count`: Number of valid pointers in `expenses`.
  - `id`: Target expense identifier.
- **Returns**: Pointer to matching `Expense` if found; `nullptr` if `id` is not present, `count <= 0`, or `expenses == nullptr`.
- **Complexity**:
  - Time: Best $O(1)$, Average $O(N)$, Worst $O(N)$.
  - Space: $O(1)$ auxiliary.

---

#### `Expense* binarySearchById(Expense* sortedExpenses[], int count, int id) const`
- **Description**: Performs iterative binary search for an expense by ID.
- **PRECONDITION (MANDATORY)**:
  - The `sortedExpenses` array **MUST be sorted in ascending order by expense ID** prior to calling this function (e.g., sorted via `Sort::sortById()`). If the input is unsorted, the binary search invariant is violated and search outcomes are undefined.
- **Parameters**:
  - `sortedExpenses`: Array of Expense pointers sorted by `id` in ascending order.
  - `count`: Number of elements in the array.
  - `id`: Target expense identifier.
- **Returns**: Pointer to matching `Expense` if found; `nullptr` if not found or if input array is invalid.
- **Complexity**:
  - Time: Best $O(1)$, Average $O(\log N)$, Worst $O(\log N)$.
  - Space: $O(1)$ auxiliary.

---

### 3.2 `Sort` Class
Header: `Sort.h`  
Namespace: Global

```cpp
class Sort {
public:
    Sort();
    ~Sort();

    void sortById(Expense* expenses[], int count) const;
    void sortByAmount(Expense* expenses[], int count) const;
    void sortByDate(Expense* expenses[], int count) const;
};
```

#### Algorithm & Implementation Details
- Implemented using **manual Quicksort** with a **middle-element pivot selection** (`pivot = arr[low + (high - low) / 2]`).
- The middle pivot guarantees that already-sorted, nearly-sorted, or reverse-sorted datasets do not trigger the classic $O(N^2)$ degradation of naive first-element pivot choices.
- **Non-destructive**: Reorders only the non-owning pointers in the supplied array. Does not modify the master linked list in `ExpenseManager` or mutate any `Expense` fields.

#### Functions

#### `void sortById(Expense* expenses[], int count) const`
- **Ordering**: Ascending numerical order of `Expense::getId()`.
- **Purpose**: Prepares an expense pointer array to satisfy the precondition of `Search::binarySearchById()`.
- **Complexity**: Time $O(N \log N)$ average/best, $O(N^2)$ worst; Space $O(\log N)$ call stack.

#### `void sortByAmount(Expense* expenses[], int count) const`
- **Ordering**: Ascending order of `Expense::getAmountPaise()`.
- **Tie-Breaker Rule**: When two expenses have identical amounts, they are ordered deterministically by `Expense::getId()` in ascending order.
- **Complexity**: Time $O(N \log N)$ average/best, $O(N^2)$ worst; Space $O(\log N)$ call stack.

#### `void sortByDate(Expense* expenses[], int count) const`
- **Ordering**: Ascending chronological order (earliest date to latest date).
- **Date Format**: Standard `"DD-MM-YYYY"`. The parser translates dates to integer keys:
  $$\text{Key} = \text{YYYY} \times 10000 + \text{MM} \times 100 + \text{DD}$$
- **Tie-Breaker Rule**: When dates are identical, tie-breaker is `Expense::getId()` ascending.
- **Complexity**: Time $O(N \log N)$ average/best, $O(N^2)$ worst; Space $O(\log N)$ call stack.

---

### 3.3 `BSTNode` Class
Header: `BSTNode.h`  
Namespace: Global

```cpp
class BSTNode {
private:
    int expenseId;
    long long amountPaise;
    BSTNode* left;
    BSTNode* right;

public:
    BSTNode();
    BSTNode(int id, long long amount);
    ~BSTNode();

    int getExpenseId() const;
    long long getAmountPaise() const;
    BSTNode* getLeft() const;
    BSTNode* getRight() const;

    void setLeft(BSTNode* newLeft);
    void setRight(BSTNode* newRight);
    void setExpenseId(int id);
    void setAmountPaise(long long amount);

    friend class ExpenseBST;
};
```

#### Invariants:
1. **Index-Only Storage**: Stores strictly `(expenseId, amountPaise)` alongside `left` and `right` child pointers.
2. **No Object Ownership**: Does **not** contain or own an `Expense` instance or pointer, preventing redundant memory usage and double-free hazards.

---

### 3.4 `ExpenseBST` Class
Header: `ExpenseBST.h`  
Namespace: Global

```cpp
class ExpenseBST {
public:
    ExpenseBST();
    ExpenseBST(const ExpenseBST& other);
    ExpenseBST& operator=(const ExpenseBST& other);
    ~ExpenseBST();

    bool insert(const Expense& expense);
    bool findByAmount(long long amountPaise, int& expenseId) const;
    void displayInOrder() const;
    void clear();

    int getCount() const;
    bool isEmpty() const;
    const BSTNode* getRoot() const;
};
```

#### Key Architecture & Traversal Rules
- **Search Key**: Composite key `(amountPaise, expenseId)`.
  - Primary ordering: `amountPaise` (ascending).
  - Secondary ordering (tie-breaker): `expenseId` (ascending).
- **Duplicate Amounts**:
  - Two expenses with the same `amountPaise` but different IDs are both stored cleanly in the tree.
  - If a incoming record has identical `amountPaise` AND identical `expenseId`, `insert()` returns `false` (exact duplicate key rejected).
- **Documented Traversal Rule for `findByAmount(amountPaise, expenseId)`**:
  - The lookup algorithm descends iteratively from `root` comparing `amountPaise`.
  - **Rule**: Returns the **first matching node encountered along the standard binary search tree path** whose `amountPaise == target`.
  - For duplicate amounts, this deterministically selects the ancestor/root node of the cluster with that amount.
  - If found: sets `expenseId = node->getExpenseId()` and returns `true`.
  - If not found: leaves `expenseId` unchanged and returns `false`.
- **`displayInOrder()`**:
  - Traverses the tree in-order (Left, Root, Right).
  - Prints all records in ascending order of amount, nicely formatted as currency (`Rs. X.YY (Z paise)`).
- **Memory Management**:
  - Destructor invokes `clear()`.
  - `clear()` performs post-order recursive deletion of all `BSTNode` allocations and resets `root = nullptr; nodeCount = 0;`.

---

## 4. Complexity Analysis Summary

| Class / Operation | Best Time | Average Time | Worst Time | Auxiliary Space | Notes |
| :--- | :---: | :---: | :---: | :---: | :--- |
| `Search::linearSearchById` | $O(1)$ | $O(N)$ | $O(N)$ | $O(1)$ | Works on unsorted inputs |
| `Search::binarySearchById` | $O(1)$ | $O(\log N)$ | $O(\log N)$ | $O(1)$ | Requires ascending ID sorted input |
| `Sort::sortById` | $O(N \log N)$ | $O(N \log N)$ | $O(N^2)$ | $O(\log N)$ | Middle-pivot Quicksort |
| `Sort::sortByAmount` | $O(N \log N)$ | $O(N \log N)$ | $O(N^2)$ | $O(\log N)$ | Tie-breaker by ID ascending |
| `Sort::sortByDate` | $O(N \log N)$ | $O(N \log N)$ | $O(N^2)$ | $O(\log N)$ | Chronological key + ID tie-breaker |
| `ExpenseBST::insert` | $O(1)$ | $O(\log N)$ | $O(N)$ | $O(1)$ | Proportional to tree height $h$ |
| `ExpenseBST::findByAmount` | $O(1)$ | $O(\log N)$ | $O(N)$ | $O(1)$ | Iterative traversal down path |
| `ExpenseBST::displayInOrder` | $O(N)$ | $O(N)$ | $O(N)$ | $O(h)$ | Recursion stack depth |
| `ExpenseBST::clear` | $O(N)$ | $O(N)$ | $O(N)$ | $O(h)$ | Post-order node deallocation |

---

## 5. Integration Contract with Person 1 (`ExpenseManager`)

### 5.1 Obtaining Non-Owning Pointer Arrays
To sort or search expenses without touching the master linked list, consumer modules (such as UI or reports) obtain a temporary pointer buffer:

```cpp
#include "ExpenseManager.h"
#include "Sort.h"
#include "Search.h"
#include "ExpenseBST.h"

void demonstrateIntegration(ExpenseManager& manager) {
    int count = manager.getCount();
    if (count <= 0) return;

    // 1. Allocate exact capacity needed with nothrow
    Expense** expenseArray = new (std::nothrow) Expense*[count];
    if (!expenseArray) {
        // Handle allocation failure gracefully
        return;
    }

    // 2. Copy non-owning pointers from ExpenseManager
    int copied = manager.copyExpensePointers(expenseArray, count);

    // 3. Sort or search temporary array
    Sort sorter;
    sorter.sortById(expenseArray, copied);

    Search searcher;
    Expense* found = searcher.binarySearchById(expenseArray, copied, 102);

    // 4. Index into ExpenseBST
    ExpenseBST bst;
    for (int i = 0; i < copied; ++i) {
        bst.insert(*expenseArray[i]);
    }

    // 5. Clean up ONLY the pointer array
    // DO NOT delete individual expenseArray[i] pointers!
    delete[] expenseArray;
    expenseArray = nullptr;
}
```

### 5.2 Ownership & Memory Safety Rules
1. **Never Call `delete expenses[i]`**: Person 1's `ExpenseManager` is the sole owner of `Expense` instances. Analysis modules only hold temporary non-owning references.
2. **Always Call `delete[] arr`**: The temporary array allocated via `new (std::nothrow) Expense*[count]` must be freed after use.
3. **Immutability of Master Linked List**: Sorting the temporary pointer array does not alter `head` or the sequence of `ExpenseNode` objects in `ExpenseManager`.

---

## 6. Compilation and Test Instructions

### 6.1 Compiling with GCC / MinGW
Run from the module directory:

```bash
g++ -Wall -Wextra -std=c++11 -O2 Expense.cpp ExpenseNode.cpp ExpenseManager.cpp Search.cpp Sort.cpp BSTNode.cpp ExpenseBST.cpp person4_test.cpp -o person4_test.exe
```

### 6.2 Running the Test Suite
```bash
./person4_test.exe
```

### 6.3 Test Coverage Summary
The test harness in `person4_test.cpp` executes **119 automated assertions** across 9 test suites:
- **Suite 1: Empty Arrays & Null Safety**: Null pointers, count = 0, negative counts.
- **Suite 2: Single Element Arrays**: Search and sorting of 1-element arrays and single-node BSTs.
- **Suite 3: Unsorted IDs & Binary Search Preconditions**: Validates that linear search succeeds on unsorted data, `sortById` produces ascending order, and binary search functions correctly with failed lookup boundaries.
- **Suite 4: Already-Sorted Data**: Validates that `sortById`, `sortByAmount`, and `sortByDate` maintain sorted state without $O(N^2)$ middle-pivot degradation.
- **Suite 5: Duplicate Amounts & Tie-Breaking**: Validates that tied amounts are consistently sub-ordered by `id` ascending.
- **Suite 6: Chronological Date Sorting**: Validates `DD-MM-YYYY` parsing across days, months, and multiple years.
- **Suite 7: BSTNode Verification**: Validates node construction, getters, setters, and absence of Expense ownership.
- **Suite 8: ExpenseBST Operations & Duplicates**: Validates duplicate amounts insertion, exact duplicate rejection, documented search traversal rule, and in-order display.
- **Suite 9: Full Integration with `ExpenseManager`**: Verifies dynamic allocation with `std::nothrow`, `copyExpensePointers()`, linked-list non-mutation, and safe array disposal.
