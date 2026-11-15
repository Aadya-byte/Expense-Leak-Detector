# PERSON 2 — Persistence, Parsing & Export Module API Specification

**Module Owner:** Person 2  
**Components:** `RecordParser`, `FileManager`, `ReportExporter`  
**Target Environment:** C++ (Strictly compliant with C++98 / C++11 / C++14; Zero STL Containers or Algorithms)  
**Verification Status:** **76 / 76 Test Assertions Passed (100% Pass Rate)**

---

## 1. Executive Summary

Person 2 is responsible for the persistence lifecycle, data serialization/deserialization, input validation, and monthly reporting of the **Expense Leak Detector** application:

1. **`RecordParser`**: Parses and strictly validates pipe-delimited records into `Expense` objects. Validates all seven fields, enforces character length constraints, validates Gregorian calendar dates (including leap year rules), validates positive non-zero amounts, and ensures compatibility between expense types and frequencies.
2. **`FileManager`**: Manages persistent disk storage (`expense.txt` in the application working folder). Employs a **two-pass transactional loading architecture** to prevent corrupt or incomplete loads from destroying valid in-memory data. Preserves original record IDs via `manager.addLoadedExpense()`, detects duplicate IDs, reports exact file line numbers for errors, safely handles missing and empty files, and uses **atomic temporary snapshots** (`.tmp`) when saving to prevent data corruption.
3. **`ReportExporter`**: Generates clean, human-readable monthly expense reports (`monthly_report_MM-YYYY.txt` in the application working folder). Traverses the singly linked list directly via `ExpenseManager::getHead()`, accumulates total spending strictly using **integer paise** (preventing floating-point inaccuracies), converts to rupees only at the presentation boundary, and isolates records matching the requested month and year.

---

## 2. Shared Data Flow & Architecture

```
+-----------------------------------------------------------------------------+
|                                User / UI                                    |
+-----------------------------------------------------------------------------+
                                       |
                                       v
+-----------------------------------------------------------------------------+
|                          ExpenseManager (Person 1)                          |
|         Single Source of Truth: Singly Linked List of Expense Records       |
+-----------------------------------------------------------------------------+
         ^                                                      |
         | (manager.addLoadedExpense)                           | (manager.getHead)
         |                                                      v
+-----------------------+                             +-----------------------+
|      FileManager      |                             |    ReportExporter     |
| (Transaction Load &   |                             | (Monthly Filtering &  |
| Atomic Snapshot Save) |                             | Integer Paise Math)   |
+-----------------------+                             +-----------------------+
         |                                                      |
         v                                                      v
+-----------------------+                             +-----------------------+
|     RecordParser      |                             | monthly_report_*.txt  |
| (Validation & Fields) |                             +-----------------------+
+-----------------------+
         |
         v
+-----------------------+
|     expense.txt      |
+-----------------------+
```

### Architectural Principles & Constraints
- **Zero STL Containers or Algorithms**: No `std::vector`, `std::list`, `std::map`, `std::set`, `std::string`, `std::sort`, or `<algorithm>`. All string operations use fixed-size char buffers and standard C string functions (`std::strlen`, `std::strncpy`, `std::strncmp`). All dynamic collections use manual dynamic arrays (`new[]` / `delete[]`) or pointers.
- **No Circular Dependencies**: Header files (`RecordParser.h`, `FileManager.h`, `ReportExporter.h`) use forward declarations (`class Expense;`, `class ExpenseManager;`) rather than including Person 1 headers directly.
- **No Duplicate Data Structures**: The persistence module does not create shadow lists or secondary arrays of expense objects. It directly interfaces with `ExpenseManager`.
- **Accurate Financial Calculations**: Money is stored and accumulated exclusively as **integer paise** (`long long`), eliminating floating-point rounding errors (`15000 paise = Rs. 150.00`).

---

## 3. Storage File Format Specification

Expense records are stored in a simple, pipe-delimited (`|`) text file:

```text
ID|Date|Amount|Merchant|Category|Type|Frequency
```

### Example Records
```text
104|15-10-2026|15000|Coffee Shop|Food|ONE_TIME|NONE
105|15-10-2026|80000|Internet|Utilities|RECURRING|MONTHLY
106|16-10-2026|4550|Metro Card|Transport|ONE_TIME|NONE
201|05-11-2026|150.75|Bakery|Food|ONE_TIME|NONE
```

### Field Specification Matrix

| Field # | Field Name | Internal Type | Format / Constraints | Rejection Criteria |
| :---: | :--- | :--- | :--- | :--- |
| **1** | `ID` | `int` | Positive integer (e.g. `104`) | `<= 0`, non-numeric, overflow (> 9 digits), empty |
| **2** | `Date` | `char[12]` | `DD-MM-YYYY` (10 chars, `-` at index 2 & 5) | Invalid format, month `< 1` or `> 12`, day invalid for month/year, non-leap Feb 29 |
| **3** | `Amount` | `long long` | Integer paise (`15000`) or decimal rupees (`150.75`) | `<= 0`, negative sign, non-numeric, > 2 decimal places |
| **4** | `Merchant`| `char[64]` | Max 63 chars (null-terminated) | Empty, all-whitespace, length >= 64, contains `\|` |
| **5** | `Category`| `char[64]` | Max 63 chars (null-terminated) | Empty, all-whitespace, length >= 64, contains `\|` |
| **6** | `Type` | `ExpenseType`| `ONE_TIME` or `RECURRING` | Any unrecognized string, empty, contains `\|` |
| **7** | `Frequency`| `ExpenseFrequency` | `NONE` (for ONE_TIME); `DAILY`, `WEEKLY`, `MONTHLY`, `YEARLY` (for RECURRING) | Mismatched type/frequency (e.g. `ONE_TIME` with `MONTHLY`), empty, contains `\|` |

---

## 4. Component API References

### 4.1. `RecordParser` Class

```cpp
#include "RecordParser.h"
```

#### Public Interface
```cpp
class RecordParser {
public:
    RecordParser();
    ~RecordParser();

    // Parses a single pipe-delimited text line into the provided Expense object.
    // Returns true on success; false if malformed or invalid.
    bool parseRecord(const char* line, Expense& outputExpense) const;

    // Validates an already instantiated Expense object against all domain rules.
    // Returns true if all fields are valid; false otherwise.
    bool validateRecord(const Expense& expense) const;
};
```

#### Detailed Behavior
- **Delimiter Counting**: Verifies that the line contains **exactly 6 pipes (`|`)**. Any line with fewer or more pipes is rejected immediately.
- **Embedded Delimiters**: Rejects any field containing embedded `|` characters.
- **Whitespace Handling**: Trims leading/trailing whitespace and `\r`/`\n` line endings from each field. Blank lines or all-whitespace strings in merchant/category are rejected.
- **Date Validation**: Performs full Gregorian calendar date checking:
  - Validates month range ($1 \le month \le 12$).
  - Validates year range ($1900 \le year \le 2100$).
  - Validates day against maximum days in that month.
  - Accounts for leap years: $((year \pmod 4 == 0 \land year \pmod{100} \neq 0) \lor year \pmod{400} == 0)$.
- **Amount Parsing**:
  - Automatically parses integer paise (`15000` $\rightarrow 15000$ paise).
  - Automatically parses decimal rupee inputs (`150.75` $\rightarrow 15075$ paise; `150.5` $\rightarrow 15050$ paise).
  - Enforces positive amount ($> 0$).
- **Type/Frequency Consistency**:
  - `ONE_TIME` expenses must have frequency `NONE` (or `N/A`).
  - `RECURRING` expenses must have frequency `DAILY`, `WEEKLY`, `MONTHLY`, or `YEARLY`.

---

### 4.2. `FileManager` Class

```cpp
#include "FileManager.h"
```

#### Public Interface
```cpp
class FileManager {
public:
    // Initializes the file manager with target storage path (defaults to "expense.txt")
    FileManager(const char* filePath = "expense.txt");
    ~FileManager();

    // Loads records from storage file into manager using transactional two-pass loading.
    bool loadExpenses(ExpenseManager& manager);

    // Saves records from manager using atomic temporary snapshot replacement.
    bool saveExpenses(const ExpenseManager& manager);

    // Returns a human-readable description of the last error encountered.
    const char* getLastError() const;

    // Path getters & setters
    const char* getFilePath() const;
    void setFilePath(const char* filePath);
};
```

#### Transactional Two-Pass Loading (`loadExpenses`)
To guarantee that valid in-memory data is never corrupted or erased by a failed or partial file load, `FileManager` executes loading in two distinct passes:

1. **Pass 1 (Validation Pass — In-Memory Manager Untouched)**:
   - Verifies the file exists and opens successfully.
   - If the file is 0 bytes (empty), it safely clears the manager to 0 records and returns `true`.
   - Iterates through each physical line in the file:
     - Tracks 1-based `lineNumber`.
     - Ignores blank lines.
     - Parses line using `RecordParser`. If parsing fails, reports exact line number (e.g., `Line 5: Malformed expense record`), leaves `manager` untouched, and returns `false`.
     - Tracks all parsed IDs in a dynamic array (`int* seenIds`) without STL.
     - Checks for duplicate IDs within the file. If a duplicate ID is found, reports exact line number and duplicate ID (e.g., `Line 8: Duplicate expense ID 104`), leaves `manager` untouched, and returns `false`.
2. **Pass 2 (Commit Pass — Safe Transactional Replacement)**:
   - Executed **only if Pass 1 completed with zero errors**.
   - Calls `manager.clear()` to release previous nodes and expenses safely.
   - Rewinds file to the beginning (`seekg(0, ios::beg)`).
   - Inserts each record via `manager.addLoadedExpense(expense)`, preserving original saved IDs without triggering automatic ID regeneration.
   - Clears `m_lastError` and returns `true`.

#### Atomic Snapshot Saving (`saveExpenses`)
- Automatically ensures the target directory exists (e.g., creates `data/` if missing).
- Writes the complete linked list snapshot to a temporary file: `<filePath>.tmp` (e.g., `expense.txt.tmp`).
- Directly traverses `manager.getHead()` without creating a second list or shadow array.
- Verifies write integrity and flushes stream before closing.
- Replaces the original file using `std::remove()` and `std::rename()`:
  - If a system crash or power outage occurs during writing, the original file remains uncorrupted.
  - If writing fails, the temporary file is deleted and the target remains untouched.

#### Error Reporting (`getLastError`)
Returns descriptive, actionable messages, including:
- `"Failed to open file for reading: 'expense.txt'"`
- `"Line 2: Malformed expense record"`
- `"Line 3: Duplicate expense ID 101"`
- `"Failed to open temporary file for writing: 'expense.txt.tmp'"`

---

### 4.3. `ReportExporter` Class

```cpp
#include "ReportExporter.h"
```

#### Public Interface
```cpp
class ReportExporter {
public:
    ReportExporter();
    ~ReportExporter();

    // Exports monthly transaction list, transaction count, and total spending.
    // Returns true on success; false on failure.
    bool exportMonthlyReport(const ExpenseManager& manager,
                             int month, int year,
                             const char* outputPath) const;
};
```

#### Detailed Behavior
- **Input Validation**:
  - $1 \le month \le 12$
  - $1900 \le year \le 2100$
  - `outputPath` must be non-null and non-empty.
- **Output Location**: The application writes `monthly_report_MM-YYYY.txt` beside the expense file.
- **Date Matching**: Extracts month and year from `DD-MM-YYYY` string and matches `(expMonth == month && expYear == year)`. Correctly distinguishes between different years (e.g., Oct 2026 vs Oct 2025).
- **Integer Paise Arithmetic**:
  - Accumulates total spending using `long long totalPaise = 0`.
  - Converts to rupees only when outputting text: `totalRupees = totalPaise / 100`, `remPaise = totalPaise % 100`.
  - Prevents binary floating-point errors (e.g. `0.1 + 0.2 != 0.3`).
- **Formatted Text Output**:
  - Simple header with full month name (e.g. `October 2026`) and transaction count.
  - Readable columnar table: `ID`, `Date`, `Amount`, `Category`, `Type`, `Frequency`, `Merchant`.
  - Currency omits unnecessary trailing zeroes (for example, `Rs. 50`, `Rs. 45.5`, and `Rs. 45.05`).
  - Empty month handling displays `No transactions recorded for this period.` and `Total spending: Rs. 0`.

#### Sample Generated Report Output
```text
MONTHLY EXPENSE REPORT
October 2026
Transactions: 4

ID       | Date       | Amount       | Category           | Type         | Frequency  | Merchant
--------+------------+--------------+--------------------+--------------+------------+----------------
101      | 05-10-2026 | Rs. 50       | Food               | ONE_TIME     | NONE       | Bakery
104      | 15-10-2026 | Rs. 150      | Food               | ONE_TIME     | NONE       | Coffee Shop
105      | 18-10-2026 | Rs. 800      | Utilities          | RECURRING    | MONTHLY    | Internet
106      | 22-10-2026 | Rs. 45.5     | Transport          | ONE_TIME     | NONE       | Metro Card
--------------------------------------------------------------------------------
Total spending: Rs. 1045.5
```

---

## 5. Team Contract & Integration Guide

### Contract Agreement with Person 1 (Core Model & Linked List)
Person 1 provides the core data structures:
- `Expense.h` / `Expense.cpp`: `Expense` class, getters, setters, enums.
- `ExpenseManager.h` / `ExpenseManager.cpp`: `ExpenseNode` linked list, `addLoadedExpense()`, `getHead()`, `getCount()`, `clear()`.

#### Required `Expense` Methods:
```cpp
int getId() const;
const char* getDate() const;
long long getAmount() const; // Integer paise
const char* getMerchant() const;
const char* getCategory() const;
ExpenseType getType() const;
const char* getTypeString() const;
ExpenseFrequency getFrequency() const;
const char* getFrequencyString() const;

void setId(int id);
void setDate(const char* date);
void setAmount(long long amountPaise);
void setMerchant(const char* merchant);
void setCategory(const char* category);
void setType(const char* typeStr);
void setFrequency(const char* freqStr);
```

#### Required `ExpenseManager` Methods:
```cpp
ExpenseNode* getHead() const;
int getCount() const;
bool addLoadedExpense(const Expense& expense);
void clear();
```

#### Required `ExpenseNode` Methods:
```cpp
Expense* getExpense() const;
ExpenseNode* getNext() const;
```

### Integration Steps for Person 6 (Main Integration)
1. Drop in Person 2's headers and implementation files:
   - `RecordParser.h`, `RecordParser.cpp`
   - `FileManager.h`, `FileManager.cpp`
   - `ReportExporter.h`, `ReportExporter.cpp`
2. Link against Person 1's `Expense.cpp` and `ExpenseManager.cpp`.
3. Discard `ExpenseTestAdapter.cpp` (it was provided solely as a test fixture for independent Person 2 verification).
4. Person 2 defines **no permanent duplicate `Expense` or `ExpenseManager` classes**, guaranteeing zero symbol redefinition or linkage collisions.

---

## 6. Error Handling & Edge Cases Matrix

| Component | Condition / Edge Case | Handled Behavior | Result |
| :--- | :--- | :--- | :--- |
| `FileManager` | File does not exist | Reports `"Failed to open file for reading..."`, does not touch `manager` | Returns `false` |
| `FileManager` | File is 0 bytes (empty) | Clears `manager` to empty dataset safely without crashing | Returns `true` |
| `FileManager` | Line has incorrect pipe count | Reports `"Line X: Malformed expense record"`, manager untouched | Returns `false` |
| `FileManager` | Line has duplicate ID | Reports `"Line X: Duplicate expense ID Y"`, manager untouched | Returns `false` |
| `FileManager` | Partial load failure on line $K$ | Pass 1 catches error; `manager` is NOT cleared and retains all previous in-memory records | Returns `false` |
| `FileManager` | Directory `data/` missing | Automatically creates parent directory using `_mkdir` / `mkdir` | Succeeds |
| `FileManager` | Save write failure / illegal path | Deletes `.tmp` snapshot, reports write failure, target untouched | Returns `false` |
| `RecordParser`| Embedded pipe `\|` in merchant | Detects $> 6$ delimiters or field corruption | Returns `false` |
| `RecordParser`| Non-leap year Feb 29 (`29-02-2023`) | Enforces Gregorian leap year rules; rejects invalid leap dates | Returns `false` |
| `RecordParser`| Negative / zero amount | Rejects amount $\le 0$ | Returns `false` |
| `RecordParser`| Decimal amount (`150.75`) | Parses whole rupees and fractional paise into integer paise (`15075`) | Returns `true` |
| `RecordParser`| Mismatched type / frequency | Rejects `ONE_TIME` with `MONTHLY` or `RECURRING` with `NONE` | Returns `false` |
| `ReportExporter`| Month with 0 transactions | Generates valid report showing 0 count, `Rs. 0.00`, and friendly message | Returns `true` |
| `ReportExporter`| Invalid month / year / path | Parameter validation check | Returns `false` |

---

## 7. Verification & Test Execution Results

The test suite in `person2_test.cpp` was compiled and executed using MinGW GCC (`g++.exe (MinGW.org GCC-6.3.0-1) 6.3.0`):

### Compilation Command
```powershell
g++ -Wall -Wextra -std=c++11 FileManager.cpp RecordParser.cpp ReportExporter.cpp ExpenseTestAdapter.cpp person2_test.cpp -o person2_test.exe
```
*Result: Compiled cleanly with zero warnings and zero errors.*

### Test Execution Output
```text
=========================================================
          PERSON 2 MODULE VERIFICATION TEST SUITE        
=========================================================

=== Test Group 1: RecordParser ===
  [PASS] Parse valid ONE_TIME expense record
  [PASS] Verify ID == 104
  [PASS] Verify Date == 15-10-2026
  [PASS] Verify Amount == 15000 paise
  [PASS] Verify Merchant == Coffee Shop
  [PASS] Verify Category == Food
  [PASS] Verify Type == EXPENSE_ONE_TIME
  [PASS] Verify Frequency == FREQ_NONE
  [PASS] Parse valid RECURRING expense record
  [PASS] Verify ID == 105
  [PASS] Verify Amount == 80000 paise
  [PASS] Verify Type == EXPENSE_RECURRING
  [PASS] Verify Frequency == FREQ_MONTHLY
  [PASS] Parse decimal currency '150.75'
  [PASS] Verify decimal converted to 15075 paise
  [PASS] Reject line with 5 pipes (too few fields)
  [PASS] Reject line with 7 pipes (too many fields)
  [PASS] Reject field containing embedded '|'
  [PASS] Reject invalid leap/month date 31-02-2026
  [PASS] Reject invalid month 15-13-2026
  [PASS] Reject invalid day 00-10-2026
  [PASS] Reject wrong delimiter 15/10/2026
  [PASS] Accept 29-02-2024 (valid leap year)
  [PASS] Reject 29-02-2023 (non-leap year)
  [PASS] Reject zero amount
  [PASS] Reject negative amount
  [PASS] Reject non-numeric amount
  [PASS] Reject unknown type
  [PASS] Reject ONE_TIME with recurring frequency MONTHLY
  [PASS] Reject RECURRING with frequency NONE
  [PASS] Reject empty merchant
  [PASS] Reject empty category
  [PASS] Reject all-whitespace merchant
  [PASS] validateRecord() returns true for valid object
  [PASS] validateRecord() rejects ID <= 0

=== Test Group 2: FileManager ===
  [PASS] loadExpenses returns false for missing file
  [PASS] getLastError contains descriptive missing file message
  [PASS] Existing in-memory data NOT erased on missing file
  [PASS] loadExpenses handles empty file safely and succeeds
  [PASS] Manager count is 0 after loading empty file
  [PASS] loadExpenses returns false on malformed file
  [PASS] Error accurately reports malformed Line 2
  [PASS] Transactional safety: manager was NOT corrupted or wiped
  [PASS] Original record 555 preserved intact
  [PASS] loadExpenses rejects duplicate ID in file
  [PASS] Error accurately reports Line 3 for duplicate
  [PASS] Error explicitly mentions duplicate ID 101
  [PASS] Manager remains clean after rejected duplicate load
  [PASS] saveExpenses writes snapshot and saves successfully
  [PASS] loadExpenses successfully reloads saved file
  [PASS] Reloaded manager has exactly 3 records
  [PASS] Preserved ID 104, amount 15000, merchant 'Coffee Shop'
  [PASS] Preserved ID 105, RECURRING, MONTHLY frequency
  [PASS] Preserved ID 106, amount 4550, category 'Transport'
  [PASS] saveExpenses returns false on write failure/invalid path
  [PASS] getLastError reports accurate write failure reason

=== Test Group 3: ReportExporter ===
  [PASS] exportMonthlyReport succeeds for October 2026
  [PASS] Generated report file exists and opened
  [PASS] Report header contains 'October 2026'
  [PASS] Report identifies exactly 4 matching transactions
  [PASS] Report calculates total spending as Rs. 1045.50
  [PASS] October transaction 'Coffee Shop' included in report table
  [PASS] October 2025 transaction 'Old Store' correctly excluded
  [PASS] exportMonthlyReport succeeds for month with 0 transactions
  [PASS] Empty month report file exists and opened
  [PASS] Report identifies 0 matching transactions
  [PASS] Report displays total spending as Rs. 0.00
  [PASS] Reject invalid month 0
  [PASS] Reject invalid month 13
  [PASS] Reject invalid year 1850
  [PASS] Reject NULL output path
  [PASS] Reject empty output path

=========================================================
                  TEST EXECUTION SUMMARY                 
=========================================================
 Total Test Assertions: 76
 Passed:                76
 Failed:                0

 >>> ALL PERSON 2 TESTS PASSED SUCCESSFULLY! <<<
=========================================================
```
