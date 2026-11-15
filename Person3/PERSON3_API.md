# PERSON 3 — Custom HashTable & Spending Leak Analyzer API Specification

**Module Owner:** Person 3  
**Components:** `HashEntry`, `HashTable`, `LeakAnalyzer`  
**Target Environment:** C++ (Strictly compliant with C++98 / C++11 / C++14; Zero STL Containers or Algorithms)  
**Verification Status:** **153 / 153 Test Assertions Passed (100% Pass Rate)**

---

## 1. Executive Summary

Person 3 owns the frequency-based spending pattern detection and custom data structures for the **Expense Leak Detector** system. The core responsibility is to aggregate financial records across arbitrary categories and merchants to identify recurring drains, repeated merchant expenses, and high-volume categories without relying on any standard template libraries (STL) or external algorithms.

The module comprises three primary components:

1. **`HashEntry`**: A self-contained, bounded storage cell encapsulating a unique grouping key (up to 127 characters), cumulative monetary totals strictly in integer paise (`long long`), transaction counts, and internal statistical trackers (minimum, maximum, and average transaction values) used to analyze spending patterns.
2. **`HashTable`**: A custom, fixed-capacity hash table implemented using an ordinary contiguous array of `HashEntry` objects. Employs the **DJB2 hash algorithm** coupled with **Open Addressing and Linear Probing** for deterministic collision resolution. Safely handles table saturation, heavy collision clustering, circular index wrap-around, duplicate keys (in-place accumulation), and oversized keys without buffer overflows.
3. **`LeakAnalyzer`**: The high-level analytical engine that performs a non-destructive, single-pass inspection of `ExpenseManager` via `getHead()` and the public `ExpenseNode` interface. Aggregates expenses by `Category:Merchant`, detects meaningful spending leaks (frequent repeated spending, recurring merchant visits), aggregates category totals, and computes monthly distributions from actual calendar dates (`DD-MM-YYYY`).

### Strict Architectural Boundaries & Policy Adherence
- **Zero STL Containers or Algorithms**: Built without `<vector>`, `<map>`, `<unordered_map>`, `<list>`, `<string>`, `<algorithm>`, or `<set>`. All buffers use fixed arrays and explicit C-string operations (`std::strlen`, `std::strncmp`, manual copies).
- **No Shared Model Re-Implementation**: Consumes `Expense` and `ExpenseManager` strictly as shared contracts (owned by Person 1). Person 3 does not implement or alter these classes.
- **Single Source of Truth**: Reads records directly via `ExpenseManager::getHead()`. Never modifies existing nodes or allocates shadow/duplicate master datasets.
- **No Predictive Forecasting or Savings Advice**: Strictly respects team contracts. Forecasting future spending and savings advice are the exclusive responsibility of Person 5.
- **Non-Labeling of High Totals as Wasteful**: High category totals (such as Rent, Tuition, or Medical bills) are explicitly recognized as high volume but **never automatically labeled as wasteful or leaks**. Leaks are defined strictly by frequency-based repeated drains.
- **Monetary Precision**: All internal financial computations are executed strictly in integer paise (`long long`), eliminating floating-point rounding errors (`Rs. 1.00 = 100 paise`). Rupees conversion occurs only at the reporting boundary.

---

## 2. Shared Data Flow & Architecture

```
+-----------------------------------------------------------------------------+
|                          ExpenseManager (Person 1)                          |
|         Single Source of Truth: Singly Linked List of Expense Records       |
+-----------------------------------------------------------------------------+
                                       |
                   Read-Only Traversal | ExpenseManager::getHead()
                                       v
+-----------------------------------------------------------------------------+
|                         LeakAnalyzer (Person 3)                             |
|  - Non-destructive traversal of ExpenseNode singly linked list              |
|  - Formulates grouping keys: "Category:Merchant"                            |
|  - Parses Gregorian dates: "DD-MM-YYYY" -> "MM-YYYY"                        |
|  - Evaluates leak detection heuristics                                      |
+-----------------------------------------------------------------------------+
         |                                     |
         | insertOrUpdate(key, amountPaise)    | insertOrUpdate(cat, amountPaise)
         v                                     v
+----------------------------------+ +----------------------------------+
|    Primary HashTable (257)       | |     Category HashTable (67)      |
|  Key: "Category:Merchant"        | |  Key: "Category"                 |
|  Collision: Linear Probing       | |  Tracks category total spend     |
|  Accumulates totals & counts     | |  Applies non-wasteful policy     |
+----------------------------------+ +----------------------------------+
         |
         | Filter: transactionCount >= 2
         v
+-----------------------------------------------------------------------------+
|               Identified Spending Patterns (m_patterns[])                   |
|  - Frequent Spending Leaks (Count >= 3)                                     |
|  - Repeated Merchant Drains (Count == 2)                                    |
+-----------------------------------------------------------------------------+
                                       |
                                       v
+-----------------------------------------------------------------------------+
|                   Formatted ASCII Leak Report (stdout)                      |
|  - Executive Summary, Patterns, Category Volume, Monthly Distribution       |
+-----------------------------------------------------------------------------+
```

---

## 3. Component Specification: `HashEntry`

### 3.1 Class Declaration (`HashEntry.h`)
```cpp
class HashEntry {
public:
    static const int MAX_KEY_LENGTH = 128;

private:
    char m_key[MAX_KEY_LENGTH];
    long long m_totalPaise;       // Total accumulated amount in integer paise
    int m_transactionCount;       // Number of transactions recorded for this key
    long long m_minPaise;         // Minimum individual transaction amount in paise
    long long m_maxPaise;         // Maximum individual transaction amount in paise
    bool m_occupied;              // True if this slot contains active data

public:
    HashEntry();
    HashEntry(const char* key, long long amountPaise);

    bool setKey(const char* key);
    const char* getKey() const;
    long long getTotalPaise() const;
    int getTransactionCount() const;
    void addTransaction(long long amountPaise);

    bool isOccupied() const;
    bool isEmpty() const;
    void clear();

    long long getMinPaise() const;
    long long getMaxPaise() const;
    long long getAveragePaise() const;
};
```

### 3.2 Method Semantics & Validation Rules

| Method | Return Type | Validation & Behavior |
| :--- | :--- | :--- |
| `HashEntry()` | N/A | Initializes all numeric members to `0`, `m_occupied` to `false`, and `m_key[0]` to `\0`. |
| `setKey(const char* key)` | `bool` | Validates `key != NULL`, `strlen(key) > 0`, and `strlen(key) < MAX_KEY_LENGTH` (128). Safely null-terminates. Returns `false` on any violation without corrupting the entry. |
| `getKey()` | `const char*` | Returns pointer to internal bounded character buffer. |
| `getTotalPaise()` | `long long` | Returns total accumulated amount in integer paise. |
| `getTransactionCount()` | `int` | Returns number of successful transactions added to this entry. |
| `addTransaction(long long amountPaise)` | `void` | Rejects non-positive amounts (`amountPaise <= 0`). Prevents 64-bit integer overflow (`LLONG_MAX - amountPaise`). Updates `m_minPaise`, `m_maxPaise`, increments count, accumulates total, and sets `m_occupied = true`. |
| `isOccupied()` / `isEmpty()` | `bool` | Returns state flag (`m_occupied` / `!m_occupied`). |
| `clear()` | `void` | Resets all fields back to initial clean state (`m_occupied = false`). |
| `getMinPaise()` / `getMaxPaise()` | `long long` | Returns recorded lowest and highest individual transaction amounts. |
| `getAveragePaise()` | `long long` | Computes `m_totalPaise / m_transactionCount`. Returns `0` if count is zero. |

---

## 4. Component Specification: `HashTable`

### 4.1 Class Declaration (`HashTable.h`)
```cpp
class HashTable {
public:
    static const int DEFAULT_CAPACITY = 257; // Prime capacity minimizes clustering

private:
    HashEntry* m_table;         // Ordinary contiguous array of HashEntry objects
    int m_capacity;             // Fixed capacity of the table
    int m_entryCount;           // Number of distinct keys stored in the table
    int m_collisionCount;       // Total number of collisions encountered

    unsigned int hashString(const char* key) const;
    static bool stringsEqual(const char* a, const char* b);

public:
    HashTable();
    explicit HashTable(int capacity);
    virtual ~HashTable();
    HashTable(const HashTable& other);
    HashTable& operator=(const HashTable& other);

    bool insertOrUpdate(const char* key, long long amountPaise);
    const HashEntry* find(const char* key) const;
    int getCapacity() const;
    int getEntryCount() const;
    const HashEntry* getEntryAt(int index) const;
    void clear();

    int getCollisionCount() const;
    bool isFull() const;
    double getLoadFactor() const;
};
```

### 4.2 Hash Function: DJB2 Algorithm
The hash table employs Daniel J. Bernstein's **DJB2** algorithm with prime multiplier:
$$\text{hash}_{i} = (\text{hash}_{i-1} \times 33) + c = ((\text{hash}_{i-1} \ll 5) + \text{hash}_{i-1}) + c$$
$$\text{index} = \text{hash} \pmod{\text{capacity}}$$

**Rationale:**
- Excellent bit dispersion for short strings with shared prefixes (e.g. `"Food:McDonalds"`, `"Food:Starbucks"`, `"Food:Subway"`).
- Computationally lightweight ($O(L)$ where $L$ is key length), avoiding costly division or floating-point instructions.

### 4.3 Collision Resolution: Open Addressing with Linear Probing
When a collision occurs at index $h = \text{hash}(key)$:
$$\text{probeIndex}(i) = (h + i) \pmod{\text{capacity}}, \quad \text{for } i = 0, 1, 2, \dots, (\text{capacity} - 1)$$

**Resolution Strategy & Invariants:**
1. **Cache Locality**: An ordinary contiguous array (`HashEntry* m_table`) ensures sequential probes access adjacent memory lines, dramatically minimizing CPU cache misses compared to chained linked lists.
2. **Circular Wrap-around**: Probing automatically wraps around slot `0` when index reaches `m_capacity - 1`.
3. **Duplicate Key Accumulation**: If a probed slot is occupied and `stringsEqual(entry.getKey(), key) == true`, the entry is updated in place via `addTransaction(amountPaise)`. `m_entryCount` is **not** incremented.
4. **New Insertion**: If an empty slot (`!isOccupied()`) is encountered, the new key is assigned, transaction added, and `m_entryCount` incremented.
5. **Full Table Handling**: If `m_entryCount >= m_capacity` or all slots are probed without finding an empty slot or key match, `insertOrUpdate()` returns `false` safely without writing out of bounds. Updating an existing key in a full table continues to succeed because it does not require a new slot.
6. **Find Invariant**: In open addressing without lazy deletion (all items are cleared collectively via `clear()`), encountering an empty slot guarantees the key does not exist in the table, terminating `find()` immediately.

---

## 5. Component Specification: `LeakAnalyzer`

### 5.1 Class Declaration (`LeakAnalyzer.h`)
```cpp
class LeakAnalyzer {
public:
    static const int MAX_PATTERNS = 256;
    static const int FREQUENT_SPENDING_THRESHOLD = 3;  // >= 3 transactions
    static const int REPEATED_SPENDING_THRESHOLD = 2;  // >= 2 transactions
    static const long long HIGH_TOTAL_PAISE_THRESHOLD = 5000000LL; // Rs. 50,000.00

private:
    HashTable m_table;                     // Grouped spending by "Category:Merchant"
    HashTable m_categoryTable;             // Spending totals grouped by "Category"
    HashTable m_monthlyTable;              // Spending totals grouped by "MM-YYYY"

    const HashEntry* m_patterns[MAX_PATTERNS]; // Pointers to identified patterns
    int m_patternCount;

    long long m_totalPaiseAnalyzed;
    int m_totalExpensesAnalyzed;

    char m_earliestDate[12];               // Format: DD-MM-YYYY
    char m_latestDate[12];                 // Format: DD-MM-YYYY

public:
    LeakAnalyzer();
    virtual ~LeakAnalyzer();

    bool analyzeExpenses(const ExpenseManager& manager);
    void displayLeakReport() const;
    int getPatternCount() const;
    const HashEntry* getPatternAt(int index) const;

    void printLeakReport(std::ostream& os) const;
    const HashTable& getHashTable() const;
    const HashTable& getCategoryTable() const;
    const HashTable& getMonthlyTable() const;
    long long getTotalAnalyzedPaise() const;
    int getTotalExpensesAnalyzed() const;
    const char* getEarliestDate() const;
    const char* getLatestDate() const;
    void clear();
};
```

### 5.2 Heuristics & Spending Pattern Rules

1. **Composite Grouping Key (`Category:Merchant`)**:
   - Expenses are grouped by `"[Category]:[Merchant]"`, providing channel-level granularity (e.g. `"Coffee:Starbucks"`, `"Transport:Uber"`).
   - If category or merchant is empty/missing, defaults to `"Uncategorized"` and `"Unknown"` respectively.

2. **Pattern Identification Criteria**:
   - **Frequent Spending Leak**: Channel has `transactionCount >= 3`. Indicates habit-driven, recurring drains (e.g., daily coffee, regular fast food).
   - **Repeated Merchant Drain**: Channel has `transactionCount == 2`. Indicates repeated visits to the same vendor.
   - **Single Transactions**: Single one-off transactions (`transactionCount == 1`) are never classified as repeated spending patterns.

3. **High Category Volume Policy (Non-Wasteful Guarantee)**:
   - Category-level totals are tracked in `m_categoryTable`.
   - If a category total exceeds `HIGH_TOTAL_PAISE_THRESHOLD` (Rs. 50,000), it is flagged with:
     > `[NOTE: High category volume. Checked as essential/discretionary per policy; not automatically labeled wasteful.]`
   - **Rationale**: High totals often stem from legitimate, unavoidable obligations (Rent, College Tuition, Hospital Bills, Taxes). Labeling them as "waste" without human context is financially unsound.

4. **Gregorian Calendar Date Analysis (`DD-MM-YYYY`)**:
   - Dates are parsed from actual transaction strings into numeric representations:
     $$\text{DateValue} = (\text{Year} \times 10000) + (\text{Month} \times 100) + \text{Day}$$
   - Accurately tracks earliest date, latest date, and aggregates monthly spending under `"MM-YYYY"`.
   - Strictly historical analysis; no predictive forecasting or future extrapolation is attempted (owned by Person 5).

---

## 6. Algorithmic Complexity Matrix

| Component | Operation | Best Case Time | Average Case Time | Worst Case Time | Space Complexity | Notes / Conditions |
| :--- | :--- | :---: | :---: | :---: | :---: | :--- |
| **`HashEntry`** | `setKey()` | $O(L)$ | $O(L)$ | $O(L)$ | $O(1)$ | $L < 128$; bounded key length check and copy |
| | `addTransaction()` | $O(1)$ | $O(1)$ | $O(1)$ | $O(1)$ | Arithmetic accumulation and min/max update |
| | `getAveragePaise()` | $O(1)$ | $O(1)$ | $O(1)$ | $O(1)$ | Single integer division |
| **`HashTable`** | `hashString()` | $O(L)$ | $O(L)$ | $O(L)$ | $O(1)$ | DJB2 hash traversal over key characters |
| | `insertOrUpdate()` | $O(1)$ | $O(1)$ | $O(N)$ | $O(1)$ | Worst case occurs when table approaches full capacity |
| | `find()` | $O(1)$ | $O(1)$ | $O(N)$ | $O(1)$ | Open addressing search; immediate exit on empty slot |
| | `clear()` | $O(C)$ | $O(C)$ | $O(C)$ | $O(1)$ | $C = \text{capacity}$; resets ordinary array in place |
| **`LeakAnalyzer`** | `analyzeExpenses()` | $O(E)$ | $O(E)$ | $O(E \times C)$ | $O(C)$ | $E = \text{expense count}$; single pass linked-list traversal |
| | `getPatternAt()` | $O(1)$ | $O(1)$ | $O(1)$ | $O(1)$ | Direct pointer array indexing |
| | `displayLeakReport()` | $O(C)$ | $O(C)$ | $O(C)$ | $O(1)$ | Traverses active slots in table to format ASCII output |

*Legend: $L = \text{key length} \le 128$, $N = \text{entry count}$, $C = \text{table capacity}$, $E = \text{number of expenses in ExpenseManager}$.*

---

## 7. Comprehensive Test Verification Suite & Actual Results

The test suite (`person3_test.cpp`) was compiled with strict compiler flags:
```bash
g++ -Wall -Wextra -pedantic -std=c++11 HashEntry.cpp HashTable.cpp LeakAnalyzer.cpp ExpenseTestAdapter.cpp person3_test.cpp -o person3_test.exe
```
**Compilation Output:** Clean compilation with **0 errors and 0 warnings**.

### 7.1 Test Execution Results

```
================================================================================
           RUNNING PERSON 3 TEST SUITE (HashTable & LeakAnalyzer)               
================================================================================
[RUN] test_HashEntry_Basics...
[RUN] test_HashEntry_Validation...
[RUN] test_HashTable_Basics...
[RUN] test_HashTable_DuplicateKeys...
[RUN] test_HashTable_Collisions_LinearProbing...
[RUN] test_HashTable_Capacity_And_Full_Table...
[RUN] test_HashTable_Inspection_And_Bounds...
[RUN] test_LeakAnalyzer_EmptyDataset...
[RUN] test_LeakAnalyzer_SpendingPatterns...
[RUN] test_LeakAnalyzer_Date_And_Monthly_Tracking...
[RUN] test_LeakAnalyzer_Report_And_Policy_Compliance...
[RUN] test_LeakAnalyzer_DisplayReportLive...
[RUN] test_ExpenseManager_Non_Destructive_Inspection...
================================================================================
TEST RESULTS: 153 / 153 ASSERTIONS PASSED
OVERALL STATUS: ALL TESTS PASSED SUCCESSFULLY! (100% PASS RATE)
================================================================================
```

### 7.2 Detailed Breakdown of Test Cases

| Test Case Name | Target Requirement | Key Assertions Verified | Result |
| :--- | :--- | :--- | :---: |
| `test_HashEntry_Basics` | `HashEntry` Core API | Initial empty state, `setKey()`, `addTransaction()`, min/max/average updates, `clear()`. | **PASS** |
| `test_HashEntry_Validation` | Key & Numeric Bounds | Rejection of `NULL`, `""`, strings $\ge 128$ chars; acceptance of 127 char boundary key; rejection of zero & negative amounts. | **PASS** |
| `test_HashTable_Basics` | Table Interface | Capacity check, entry count tracking, single key insertion, successful `find()`, non-existent key returns `NULL`. | **PASS** |
| `test_HashTable_DuplicateKeys` | Repeated Key Handling | Multiple inserts of `"Groceries:Walmart"`; verifies entry count stays `1`; verifies paise accumulation and statistics updates. | **PASS** |
| `test_HashTable_Collisions` | Linear Probing | Small capacity (7) table; forces deliberate collisions; verifies collision tracking and successful retrieval of all keys. | **PASS** |
| `test_HashTable_Capacity` | Full Table Safeguards | Fills table to capacity (5/5); verifies 6th distinct insert returns `false`; verifies updating existing key in full table succeeds. | **PASS** |
| `test_HashTable_Inspection` | Bounds & Deep Copy | Out-of-bounds `getEntryAt(-1, 100)` returns `NULL`; copy constructor creates independent deep copy. | **PASS** |
| `test_LeakAnalyzer_EmptyDataset` | Safe Empty Handling | Passing empty `ExpenseManager` returns `true`, 0 expenses analyzed, 0 patterns; report prints safely without crashing. | **PASS** |
| `test_LeakAnalyzer_Patterns` | Leak Pattern Heuristics | 3x Starbucks (Frequent Leak), 2x Uber (Repeated Drain), 1x Landlord (Single Rent Tx, not leak); verifies exact pattern count (2). | **PASS** |
| `test_LeakAnalyzer_Dates` | Date & Monthly Analysis | Parses `"15-08-2026"`, `"02-09-2026"`, `"28-10-2026"`; accurately computes earliest/latest date and monthly table entries. | **PASS** |
| `test_LeakAnalyzer_Policy` | Policy Compliance | High tuition expense verified; confirms report displays policy disclaimers and excludes forecasting/advice. | **PASS** |
| `test_LeakAnalyzer_Display` | Visual ASCII Output | Executes live formatted report to stdout; validates layout, currency formatting, and section headers. | **PASS** |
| `test_ExpenseManager_Safety` | Non-Destructive Invariant | Verifies `ExpenseManager` node count and expense fields are completely unchanged after analysis. | **PASS** |

---

## 8. Edge Cases & Safety Matrix

| Edge Case Scenario | Potential Failure Mode | Implemented Mitigation & Safeguard | Verification Test |
| :--- | :--- | :--- | :--- |
| **Table Saturation** | Buffer overrun or infinite loop | `insertOrUpdate()` checks `m_entryCount >= m_capacity` and loop terminates after `m_capacity` probes returning `false`. | `test_HashTable_Capacity_And_Full_Table` |
| **Repeated Key on Full Table** | Rejecting valid update | Existing keys are checked before empty-slot insertion. Updates succeed even when table is at 100% capacity. | `test_HashTable_Capacity_And_Full_Table` |
| **Circular Index Wrap** | Access out of table bounds | All probe steps use `(initialIndex + step) % m_capacity`. | `test_HashTable_Collisions_LinearProbing` |
| **64-bit Integer Overflow** | Wrapped monetary totals | `addTransaction()` verifies `m_totalPaise <= LLONG_MAX - amountPaise` prior to addition. | `test_HashEntry_Basics` |
| **Empty or NULL Inputs** | Segfault or dereferencing null | Explicit `NULL` and length checks on all string parameters across all public APIs. | `test_HashEntry_Validation`, `test_HashTable_Basics` |
| **Overlength Keys ($> 127$ chars)** | Buffer overrun | `setKey()` enforces bounded length `< 128` characters; oversized keys rejected safely. | `test_HashEntry_Validation` |
| **Empty Master Dataset** | Crash on null pointer | `analyzeExpenses()` checks `manager.getHead() == NULL`, resets state, and returns `true`. | `test_LeakAnalyzer_EmptyDataset` |
| **High Expense Amounts** | False positive "waste" flag | Policy engine restricts pattern detection to frequency ($\ge 2$). High totals flagged as high volume, not waste. | `test_LeakAnalyzer_Report_And_Policy_Compliance` |
