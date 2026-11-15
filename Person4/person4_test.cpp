#include <iostream>
#include <new>

// Person 4 Headers
#include "Search.h"
#include "Sort.h"
#include "BSTNode.h"
#include "ExpenseBST.h"

// Person 1 Shared Headers
#include "Expense.h"
#include "ExpenseTypes.h"
#include "ExpenseManager.h"

// Simple testing harness without STL or external frameworks
static int testsPassed = 0;
static int testsFailed = 0;

#define ASSERT_TRUE(condition, testName) \
    do { \
        if (condition) { \
            std::cout << "  [PASS] " << testName << "\n"; \
            testsPassed++; \
        } else { \
            std::cout << "  [FAIL] " << testName << " (Line " << __LINE__ << ")\n"; \
            testsFailed++; \
        } \
    } while (0)

#define ASSERT_FALSE(condition, testName) \
    ASSERT_TRUE(!(condition), testName)

#define ASSERT_EQ(actual, expected, testName) \
    ASSERT_TRUE((actual) == (expected), testName)

// ============================================================================
// TEST 1: Empty Array Handling
// ============================================================================
void testEmptyArrays() {
    std::cout << "\n=== Running Test Suite 1: Empty Arrays & Null Safety ===\n";

    Search searcher;
    Sort sorter;

    // 1.1 Null pointers with count = 0
    ASSERT_EQ(searcher.linearSearchById(nullptr, 0, 101), nullptr, "linearSearch on nullptr array");
    ASSERT_EQ(searcher.binarySearchById(nullptr, 0, 101), nullptr, "binarySearch on nullptr array");

    // 1.2 Empty array (non-null pointer, count = 0)
    Expense* emptyArr[1] = { nullptr };
    ASSERT_EQ(searcher.linearSearchById(emptyArr, 0, 101), nullptr, "linearSearch on count=0 array");
    ASSERT_EQ(searcher.binarySearchById(emptyArr, 0, 101), nullptr, "binarySearch on count=0 array");

    // 1.3 Negative count / invalid size
    ASSERT_EQ(searcher.linearSearchById(emptyArr, -5, 101), nullptr, "linearSearch on negative count");
    ASSERT_EQ(searcher.binarySearchById(emptyArr, -5, 101), nullptr, "binarySearch on negative count");

    // 1.4 Sorting empty and null arrays (must not crash)
    sorter.sortById(nullptr, 0);
    sorter.sortByAmount(nullptr, 0);
    sorter.sortByDate(nullptr, 0);
    sorter.sortById(emptyArr, 0);
    sorter.sortByAmount(emptyArr, 0);
    sorter.sortByDate(emptyArr, 0);
    ASSERT_TRUE(true, "Sort functions safely handle null and empty arrays without crashing");
}

// ============================================================================
// TEST 2: Single Element Handling
// ============================================================================
void testSingleElement() {
    std::cout << "\n=== Running Test Suite 2: Single Element ===\n";

    Search searcher;
    Sort sorter;

    Expense single(101, "15-10-2026", 15000, "Coffee Shop", ExpenseCategory::FOOD, ExpenseType::ONE_TIME, ExpenseFrequency::NONE);
    Expense* arr[1] = { &single };

    // Search tests
    ASSERT_EQ(searcher.linearSearchById(arr, 1, 101), &single, "linearSearch finds existing single element");
    ASSERT_EQ(searcher.linearSearchById(arr, 1, 999), nullptr, "linearSearch returns nullptr for missing element");
    ASSERT_EQ(searcher.binarySearchById(arr, 1, 101), &single, "binarySearch finds existing single element");
    ASSERT_EQ(searcher.binarySearchById(arr, 1, 50), nullptr, "binarySearch returns nullptr for smaller ID");
    ASSERT_EQ(searcher.binarySearchById(arr, 1, 200), nullptr, "binarySearch returns nullptr for larger ID");

    // Sort tests (should leave array intact)
    sorter.sortById(arr, 1);
    ASSERT_EQ(arr[0]->getId(), 101, "sortById preserves single element array");
    sorter.sortByAmount(arr, 1);
    ASSERT_EQ(arr[0]->getId(), 101, "sortByAmount preserves single element array");
    sorter.sortByDate(arr, 1);
    ASSERT_EQ(arr[0]->getId(), 101, "sortByDate preserves single element array");

    // BST Single Element
    ExpenseBST bst;
    ASSERT_TRUE(bst.insert(single), "BST insert single element");
    ASSERT_EQ(bst.getCount(), 1, "BST count is 1");
    int foundId = 0;
    ASSERT_TRUE(bst.findByAmount(15000, foundId), "BST findByAmount succeeds on single element");
    ASSERT_EQ(foundId, 101, "BST findByAmount returns correct expense ID");
    ASSERT_FALSE(bst.findByAmount(9999, foundId), "BST findByAmount returns false for non-existent amount");
}

// ============================================================================
// TEST 3: Unsorted IDs vs. Sorted IDs & Binary Search Precondition
// ============================================================================
void testUnsortedAndBinarySearchPrecondition() {
    std::cout << "\n=== Running Test Suite 3: Unsorted IDs & Binary Search Precondition ===\n";

    Search searcher;
    Sort sorter;

    Expense e1(105, "10-10-2026", 12000, "Grocery", ExpenseCategory::FOOD, ExpenseType::ONE_TIME, ExpenseFrequency::NONE);
    Expense e2(102, "11-10-2026", 45000, "Bus Pass", ExpenseCategory::TRANSPORT, ExpenseType::RECURRING, ExpenseFrequency::MONTHLY);
    Expense e3(109, "12-10-2026", 3000,  "Snack", ExpenseCategory::FOOD, ExpenseType::ONE_TIME, ExpenseFrequency::NONE);
    Expense e4(101, "13-10-2026", 80000, "Electricity", ExpenseCategory::UTILITIES, ExpenseType::RECURRING, ExpenseFrequency::MONTHLY);
    Expense e5(107, "14-10-2026", 1500,  "Tea", ExpenseCategory::FOOD, ExpenseType::ONE_TIME, ExpenseFrequency::NONE);
    Expense e6(103, "15-10-2026", 25000, "Movie", ExpenseCategory::ENTERTAINMENT, ExpenseType::ONE_TIME, ExpenseFrequency::NONE);

    Expense* arr[6] = { &e1, &e2, &e3, &e4, &e5, &e6 };
    int count = 6;

    // Linear search works on unsorted array
    ASSERT_EQ(searcher.linearSearchById(arr, count, 105), &e1, "linearSearch finds e1 (105) in unsorted array");
    ASSERT_EQ(searcher.linearSearchById(arr, count, 101), &e4, "linearSearch finds e4 (101) in unsorted array");
    ASSERT_EQ(searcher.linearSearchById(arr, count, 103), &e6, "linearSearch finds e6 (103) in unsorted array");
    ASSERT_EQ(searcher.linearSearchById(arr, count, 999), nullptr, "linearSearch returns nullptr for missing ID 999");

    // Sort by ID to satisfy binary search precondition
    sorter.sortById(arr, count);

    // Verify array is sorted by ID ascending: 101, 102, 103, 105, 107, 109
    bool isSorted = true;
    for (int i = 0; i < count - 1; ++i) {
        if (arr[i]->getId() > arr[i + 1]->getId()) {
            isSorted = false;
            break;
        }
    }
    ASSERT_TRUE(isSorted, "sortById successfully sorted array in ascending order of ID");
    ASSERT_EQ(arr[0]->getId(), 101, "First element has smallest ID 101");
    ASSERT_EQ(arr[count - 1]->getId(), 109, "Last element has largest ID 109");

    // Now test binary search on sorted array
    ASSERT_EQ(searcher.binarySearchById(arr, count, 101), &e4, "binarySearch finds smallest ID 101");
    ASSERT_EQ(searcher.binarySearchById(arr, count, 102), &e2, "binarySearch finds ID 102");
    ASSERT_EQ(searcher.binarySearchById(arr, count, 103), &e6, "binarySearch finds ID 103");
    ASSERT_EQ(searcher.binarySearchById(arr, count, 105), &e1, "binarySearch finds ID 105");
    ASSERT_EQ(searcher.binarySearchById(arr, count, 107), &e5, "binarySearch finds ID 107");
    ASSERT_EQ(searcher.binarySearchById(arr, count, 109), &e3, "binarySearch finds largest ID 109");

    // Failed lookups
    ASSERT_EQ(searcher.binarySearchById(arr, count, 100), nullptr, "binarySearch returns nullptr for ID < min");
    ASSERT_EQ(searcher.binarySearchById(arr, count, 104), nullptr, "binarySearch returns nullptr for missing intermediate ID 104");
    ASSERT_EQ(searcher.binarySearchById(arr, count, 110), nullptr, "binarySearch returns nullptr for ID > max");
}

// ============================================================================
// TEST 4: Already-Sorted Data
// ============================================================================
void testAlreadySortedData() {
    std::cout << "\n=== Running Test Suite 4: Already-Sorted Data ===\n";

    Sort sorter;
    Search searcher;

    Expense e1(1, "01-01-2025", 1000, "Item 1", ExpenseCategory::FOOD, ExpenseType::ONE_TIME, ExpenseFrequency::NONE);
    Expense e2(2, "02-02-2025", 2000, "Item 2", ExpenseCategory::FOOD, ExpenseType::ONE_TIME, ExpenseFrequency::NONE);
    Expense e3(3, "03-03-2025", 3000, "Item 3", ExpenseCategory::FOOD, ExpenseType::ONE_TIME, ExpenseFrequency::NONE);
    Expense e4(4, "04-04-2025", 4000, "Item 4", ExpenseCategory::FOOD, ExpenseType::ONE_TIME, ExpenseFrequency::NONE);
    Expense e5(5, "05-05-2025", 5000, "Item 5", ExpenseCategory::FOOD, ExpenseType::ONE_TIME, ExpenseFrequency::NONE);

    Expense* arr[5] = { &e1, &e2, &e3, &e4, &e5 };

    // Already sorted by ID
    sorter.sortById(arr, 5);
    for (int i = 0; i < 5; ++i) {
        ASSERT_EQ(arr[i]->getId(), i + 1, "sortById maintains order on already-sorted input");
    }

    // Already sorted by Amount
    sorter.sortByAmount(arr, 5);
    for (int i = 0; i < 5; ++i) {
        ASSERT_EQ(arr[i]->getAmountPaise(), (i + 1) * 1000, "sortByAmount maintains order on already-sorted input");
    }

    // Already sorted by Date
    sorter.sortByDate(arr, 5);
    ASSERT_EQ(arr[0]->getId(), 1, "sortByDate maintains earliest date on already-sorted input");
    ASSERT_EQ(arr[4]->getId(), 5, "sortByDate maintains latest date on already-sorted input");
}

// ============================================================================
// TEST 5: Duplicate Amounts & Tie-Breaking in Sort
// ============================================================================
void testDuplicateAmountsInSort() {
    std::cout << "\n=== Running Test Suite 5: Duplicate Amounts & Tie-Breaking in Sort ===\n";

    Sort sorter;

    Expense e1(105, "15-10-2026", 25000, "Restaurant", ExpenseCategory::FOOD, ExpenseType::ONE_TIME, ExpenseFrequency::NONE);
    Expense e2(102, "15-10-2026", 25000, "Cafe", ExpenseCategory::FOOD, ExpenseType::ONE_TIME, ExpenseFrequency::NONE);
    Expense e3(108, "15-10-2026", 10000, "Taxi", ExpenseCategory::TRANSPORT, ExpenseType::ONE_TIME, ExpenseFrequency::NONE);
    Expense e4(101, "15-10-2026", 25000, "Supermarket", ExpenseCategory::SHOPPING, ExpenseType::ONE_TIME, ExpenseFrequency::NONE);
    Expense e5(103, "15-10-2026", 50000, "Flight", ExpenseCategory::TRANSPORT, ExpenseType::ONE_TIME, ExpenseFrequency::NONE);

    Expense* arr[5] = { &e1, &e2, &e3, &e4, &e5 };

    sorter.sortByAmount(arr, 5);

    // Expected order:
    // Index 0: 10000 paise (ID 108)
    // Index 1, 2, 3: 25000 paise (tied amounts broken by ID ascending: 101, then 102, then 105)
    // Index 4: 50000 paise (ID 103)
    ASSERT_EQ(arr[0]->getAmountPaise(), 10000, "Index 0 has amount 10000");
    ASSERT_EQ(arr[0]->getId(), 108, "Index 0 has ID 108");

    ASSERT_EQ(arr[1]->getAmountPaise(), 25000, "Index 1 has amount 25000");
    ASSERT_EQ(arr[1]->getId(), 101, "Index 1 has tied amount broken by ID 101");

    ASSERT_EQ(arr[2]->getAmountPaise(), 25000, "Index 2 has amount 25000");
    ASSERT_EQ(arr[2]->getId(), 102, "Index 2 has tied amount broken by ID 102");

    ASSERT_EQ(arr[3]->getAmountPaise(), 25000, "Index 3 has amount 25000");
    ASSERT_EQ(arr[3]->getId(), 105, "Index 3 has tied amount broken by ID 105");

    ASSERT_EQ(arr[4]->getAmountPaise(), 50000, "Index 4 has amount 50000");
    ASSERT_EQ(arr[4]->getId(), 103, "Index 4 has ID 103");
}

// ============================================================================
// TEST 6: Date Sorting Across Months, Years, and Formats
// ============================================================================
void testDateSorting() {
    std::cout << "\n=== Running Test Suite 6: Chronological Date Sorting ===\n";

    Sort sorter;

    Expense e1(1, "15-10-2026", 5000, "A", ExpenseCategory::OTHER, ExpenseType::ONE_TIME, ExpenseFrequency::NONE);
    Expense e2(2, "05-02-2025", 6000, "B", ExpenseCategory::OTHER, ExpenseType::ONE_TIME, ExpenseFrequency::NONE);
    Expense e3(3, "28-02-2026", 7000, "C", ExpenseCategory::OTHER, ExpenseType::ONE_TIME, ExpenseFrequency::NONE);
    Expense e4(4, "15-10-2025", 8000, "D", ExpenseCategory::OTHER, ExpenseType::ONE_TIME, ExpenseFrequency::NONE);
    Expense e5(5, "01-01-2025", 9000, "E", ExpenseCategory::OTHER, ExpenseType::ONE_TIME, ExpenseFrequency::NONE);

    Expense* arr[5] = { &e1, &e2, &e3, &e4, &e5 };

    sorter.sortByDate(arr, 5);

    // Expected chronological order:
    // 1: 01-01-2025 (ID 5)
    // 2: 05-02-2025 (ID 2)
    // 3: 15-10-2025 (ID 4)
    // 4: 28-02-2026 (ID 3)
    // 5: 15-10-2026 (ID 1)
    ASSERT_EQ(arr[0]->getId(), 5, "Earliest date is 01-01-2025 (ID 5)");
    ASSERT_EQ(arr[1]->getId(), 2, "Second date is 05-02-2025 (ID 2)");
    ASSERT_EQ(arr[2]->getId(), 4, "Third date is 15-10-2025 (ID 4)");
    ASSERT_EQ(arr[3]->getId(), 3, "Fourth date is 28-02-2026 (ID 3)");
    ASSERT_EQ(arr[4]->getId(), 1, "Latest date is 15-10-2026 (ID 1)");
}

// ============================================================================
// TEST 7: BSTNode Verification
// ============================================================================
void testBSTNode() {
    std::cout << "\n=== Running Test Suite 7: BSTNode Verification ===\n";

    BSTNode defaultNode;
    ASSERT_EQ(defaultNode.getExpenseId(), 0, "Default BSTNode expenseId is 0");
    ASSERT_EQ(defaultNode.getAmountPaise(), 0, "Default BSTNode amountPaise is 0");
    ASSERT_EQ(defaultNode.getLeft(), nullptr, "Default BSTNode left is nullptr");
    ASSERT_EQ(defaultNode.getRight(), nullptr, "Default BSTNode right is nullptr");

    BSTNode paramNode(42, 125000);
    ASSERT_EQ(paramNode.getExpenseId(), 42, "Param BSTNode expenseId is 42");
    ASSERT_EQ(paramNode.getAmountPaise(), 125000, "Param BSTNode amountPaise is 125000");

    BSTNode leftChild(10, 5000);
    BSTNode rightChild(20, 15000);
    paramNode.setLeft(&leftChild);
    paramNode.setRight(&rightChild);
    ASSERT_EQ(paramNode.getLeft(), &leftChild, "BSTNode setLeft / getLeft operates correctly");
    ASSERT_EQ(paramNode.getRight(), &rightChild, "BSTNode setRight / getRight operates correctly");
    paramNode.setLeft(nullptr);
    paramNode.setRight(nullptr);
}

// ============================================================================
// TEST 8: ExpenseBST Duplicate Amounts, Search Key, and Traversal Rules
// ============================================================================
void testExpenseBSTOperations() {
    std::cout << "\n=== Running Test Suite 8: ExpenseBST Operations & Duplicates ===\n";

    ExpenseBST bst;

    ASSERT_TRUE(bst.isEmpty(), "BST starts empty");
    ASSERT_EQ(bst.getCount(), 0, "BST count starts at 0");

    // Test find on empty tree
    int queryId = -1;
    ASSERT_FALSE(bst.findByAmount(10000, queryId), "findByAmount on empty tree returns false");
    ASSERT_EQ(queryId, -1, "queryId untouched when not found");

    // Insert records with duplicate amounts
    Expense e1(102, "01-10-2026", 50000, "Electronics", ExpenseCategory::SHOPPING, ExpenseType::ONE_TIME, ExpenseFrequency::NONE);
    Expense e2(101, "02-10-2026", 50000, "Appliances", ExpenseCategory::SHOPPING, ExpenseType::ONE_TIME, ExpenseFrequency::NONE);
    Expense e3(103, "03-10-2026", 50000, "Gadgets", ExpenseCategory::SHOPPING, ExpenseType::ONE_TIME, ExpenseFrequency::NONE);
    Expense e4(104, "04-10-2026", 20000, "Books", ExpenseCategory::ENTERTAINMENT, ExpenseType::ONE_TIME, ExpenseFrequency::NONE);
    Expense e5(105, "05-10-2026", 80000, "Rent", ExpenseCategory::UTILITIES, ExpenseType::RECURRING, ExpenseFrequency::MONTHLY);

    ASSERT_TRUE(bst.insert(e1), "Insert root (amount 50000, id 102)");
    ASSERT_TRUE(bst.insert(e2), "Insert duplicate amount (50000, id 101 < 102 -> placed in left subtree)");
    ASSERT_TRUE(bst.insert(e3), "Insert duplicate amount (50000, id 103 > 102 -> placed in right subtree)");
    ASSERT_TRUE(bst.insert(e4), "Insert smaller amount (20000, id 104)");
    ASSERT_TRUE(bst.insert(e5), "Insert larger amount (80000, id 105)");

    ASSERT_EQ(bst.getCount(), 5, "BST contains 5 nodes");
    ASSERT_FALSE(bst.isEmpty(), "BST is not empty");

    // Attempting to insert exact duplicate key (same amount and same ID) must fail
    ASSERT_FALSE(bst.insert(e1), "Inserting exact duplicate key (50000, 102) returns false");

    // Search by amount
    // According to documented traversal rule, findByAmount returns the first node encountered
    // along the search path with amount == 50000, which is the root (id 102)
    int foundId = 0;
    ASSERT_TRUE(bst.findByAmount(50000, foundId), "findByAmount finds amount 50000");
    ASSERT_EQ(foundId, 102, "Documented traversal rule returns root ID 102 as first matching record");

    ASSERT_TRUE(bst.findByAmount(20000, foundId), "findByAmount finds amount 20000");
    ASSERT_EQ(foundId, 104, "findByAmount returns ID 104 for 20000");

    ASSERT_TRUE(bst.findByAmount(80000, foundId), "findByAmount finds amount 80000");
    ASSERT_EQ(foundId, 105, "findByAmount returns ID 105 for 80000");

    // Failed search
    ASSERT_FALSE(bst.findByAmount(99999, foundId), "findByAmount returns false for non-existent amount");

    // Test in-order display
    std::cout << "\n  [Testing In-Order Traversal Output]:\n";
    bst.displayInOrder();

    // Test clear
    bst.clear();
    ASSERT_TRUE(bst.isEmpty(), "BST is empty after clear()");
    ASSERT_EQ(bst.getCount(), 0, "BST count is 0 after clear()");
    ASSERT_FALSE(bst.findByAmount(50000, foundId), "findByAmount returns false after clear");
}

// ============================================================================
// TEST 9: Full Integration with ExpenseManager & copyExpensePointers()
// ============================================================================
void testIntegrationWithExpenseManager() {
    std::cout << "\n=== Running Test Suite 9: ExpenseManager Integration ===\n";

    ExpenseManager manager;

    // Add expenses to master linked list
    manager.addExpense(new Expense(0, "15-10-2026", 15000, "Coffee Shop", ExpenseCategory::FOOD, ExpenseType::ONE_TIME, ExpenseFrequency::NONE));
    manager.addExpense(new Expense(0, "10-10-2026", 80000, "Internet", ExpenseCategory::UTILITIES, ExpenseType::RECURRING, ExpenseFrequency::MONTHLY));
    manager.addExpense(new Expense(0, "01-10-2026", 25000, "Dinner", ExpenseCategory::FOOD, ExpenseType::ONE_TIME, ExpenseFrequency::NONE));
    manager.addExpense(new Expense(0, "20-10-2026", 3000,  "Metro", ExpenseCategory::TRANSPORT, ExpenseType::ONE_TIME, ExpenseFrequency::NONE));
    manager.addExpense(new Expense(0, "05-10-2026", 25000, "Groceries", ExpenseCategory::SHOPPING, ExpenseType::ONE_TIME, ExpenseFrequency::NONE));

    int count = manager.getCount();
    ASSERT_EQ(count, 5, "ExpenseManager has 5 expenses");

    // Requirement:
    // "Use ExpenseManager::copyExpensePointers() to obtain an array for search and sorting,
    // allocating only the capacity needed for the current count and checking allocation failure."
    // "Do not store or delete the original Expense pointers."
    Expense** arr = nullptr;
    if (count > 0) {
        arr = new (std::nothrow) Expense*[count];
    }

    ASSERT_TRUE(arr != nullptr, "Successfully allocated non-owning array of capacity 5");

    if (arr) {
        int copied = manager.copyExpensePointers(arr, count);
        ASSERT_EQ(copied, count, "copyExpensePointers copied all 5 pointers");

        // Verify pointers point to valid records
        for (int i = 0; i < count; ++i) {
            ASSERT_TRUE(arr[i] != nullptr, "Copied pointer is valid and non-null");
        }

        // Sort the non-owning array by ID
        Sort sorter;
        sorter.sortById(arr, count);

        // Verify array is sorted by ID ascending (IDs 1, 2, 3, 4, 5)
        for (int i = 0; i < count; ++i) {
            ASSERT_EQ(arr[i]->getId(), i + 1, "arr[i] sorted by ID matches expected sequential ID");
        }

        // Binary search on sorted array
        Search searcher;
        Expense* foundExp = searcher.binarySearchById(arr, count, 3);
        ASSERT_TRUE(foundExp != nullptr, "binarySearch found expense ID 3");
        if (foundExp) {
            ASSERT_EQ(foundExp->getId(), 3, "Found expense ID is 3");
            ASSERT_EQ(foundExp->getAmountPaise(), 25000, "Found expense amount is 25000");
        }

        // Test sorting by Amount
        sorter.sortByAmount(arr, count);
        ASSERT_EQ(arr[0]->getAmountPaise(), 3000, "Lowest expense is 3000 paise");
        ASSERT_EQ(arr[count - 1]->getAmountPaise(), 80000, "Highest expense is 80000 paise");

        // Index the expenses into ExpenseBST
        ExpenseBST bst;
        for (int i = 0; i < count; ++i) {
            bst.insert(*arr[i]);
        }
        ASSERT_EQ(bst.getCount(), 5, "BST indexed all 5 expenses");

        int bstFoundId = 0;
        ASSERT_TRUE(bst.findByAmount(80000, bstFoundId), "BST findByAmount 80000 succeeds");
        ASSERT_EQ(bstFoundId, 2, "BST findByAmount returns ID 2 (Internet bill)");

        std::cout << "\n  [Displaying Integrated BST in-order]:\n";
        bst.displayInOrder();

        // Requirement: "Do not store or delete the original Expense pointers."
        // We delete only the allocated pointer array 'arr', NOT the objects it points to!
        delete[] arr;
        arr = nullptr;
    }

    // Verify ExpenseManager's internal linked list was not corrupted or modified
    ASSERT_EQ(manager.getCount(), 5, "ExpenseManager record count remains 5");
    Expense* originalExp = manager.findExpenseById(1);
    ASSERT_TRUE(originalExp != nullptr, "Expense 1 still exists in ExpenseManager");
    if (originalExp) {
        ASSERT_EQ(originalExp->getId(), 1, "Expense 1 ID intact");
        ASSERT_EQ(originalExp->getAmountPaise(), 15000, "Expense 1 amount intact");
    }

    // Clean up master manager
    manager.clear();
    ASSERT_EQ(manager.getCount(), 0, "ExpenseManager cleared safely without double-free");
}

// ============================================================================
// MAIN TEST DRIVER
// ============================================================================
int main() {
    std::cout << "============================================================\n";
    std::cout << "  PERSON 4 TEST SUITE: Search, Sort and ExpenseBST\n";
    std::cout << "============================================================\n";

    testEmptyArrays();
    testSingleElement();
    testUnsortedAndBinarySearchPrecondition();
    testAlreadySortedData();
    testDuplicateAmountsInSort();
    testDateSorting();
    testBSTNode();
    testExpenseBSTOperations();
    testIntegrationWithExpenseManager();

    std::cout << "\n============================================================\n";
    std::cout << "  TEST SUMMARY\n";
    std::cout << "============================================================\n";
    std::cout << "  Total Assertions Passed: " << testsPassed << "\n";
    std::cout << "  Total Assertions Failed: " << testsFailed << "\n";

    if (testsFailed == 0) {
        std::cout << "  >>> ALL TESTS PASSED SUCCESSFULLY! <<<\n";
    } else {
        std::cout << "  >>> SOME TESTS FAILED! <<<\n";
    }
    std::cout << "============================================================\n";

    return (testsFailed == 0) ? 0 : 1;
}
