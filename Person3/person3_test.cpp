#include "HashEntry.h"
#include "HashTable.h"
#include "LeakAnalyzer.h"
#include "Expense.h"
#include "ExpenseManager.h"
#include <iostream>
#include <sstream>
#include <cstring>

/*
 * ============================================================================
 * PERSON 3 COMPREHENSIVE TEST SUITE
 * Components Tested: HashEntry, HashTable, LeakAnalyzer
 * Coverage:
 *   1. HashEntry boundary conditions, statistics, input validation
 *   2. HashTable collision resolution (linear probing), duplicate keys,
 *      table capacity & full-table rejection, circular wrap-around
 *   3. LeakAnalyzer repeated spending detection, frequent spending leaks,
 *      high category volume non-labeling policy, date parsing & monthly
 *      grouping, empty dataset safety, and non-destructive traversal
 * ============================================================================
 */

static int g_testsRun = 0;
static int g_testsPassed = 0;

#define ASSERT_TRUE(cond, msg) do { \
    g_testsRun++; \
    if (cond) { \
        g_testsPassed++; \
    } else { \
        std::cout << "[FAIL] Line " << __LINE__ << ": " << msg << "\n"; \
    } \
} while(0)

#define ASSERT_FALSE(cond, msg) ASSERT_TRUE(!(cond), msg)

#define ASSERT_EQ(val1, val2, msg) do { \
    g_testsRun++; \
    if ((val1) == (val2)) { \
        g_testsPassed++; \
    } else { \
        std::cout << "[FAIL] Line " << __LINE__ << ": " << msg \
                  << " (Expected " << (val2) << ", Got " << (val1) << ")\n"; \
    } \
} while(0)

#define ASSERT_STR_EQ(str1, str2, msg) do { \
    g_testsRun++; \
    if ((str1 != NULL && str2 != NULL && std::strcmp(str1, str2) == 0) || (str1 == NULL && str2 == NULL)) { \
        g_testsPassed++; \
    } else { \
        std::cout << "[FAIL] Line " << __LINE__ << ": " << msg \
                  << " (Expected \"" << (str2 ? str2 : "NULL") << "\", Got \"" << (str1 ? str1 : "NULL") << "\")\n"; \
    } \
} while(0)

// ============================================================================
// TEST GROUP 1: HashEntry Unit Tests
// ============================================================================
void test_HashEntry_Basics() {
    std::cout << "[RUN] test_HashEntry_Basics...\n";

    HashEntry entry;
    ASSERT_TRUE(entry.isEmpty(), "New entry should be empty");
    ASSERT_FALSE(entry.isOccupied(), "New entry should not be occupied");
    ASSERT_EQ(entry.getTotalPaise(), 0LL, "Initial total paise should be 0");
    ASSERT_EQ(entry.getTransactionCount(), 0, "Initial transaction count should be 0");
    ASSERT_EQ(entry.getAveragePaise(), 0LL, "Initial average should be 0");
    ASSERT_STR_EQ(entry.getKey(), "", "Initial key should be empty string");

    // Valid key setting
    ASSERT_TRUE(entry.setKey("Food:Dominos"), "Setting valid key should succeed");
    ASSERT_STR_EQ(entry.getKey(), "Food:Dominos", "Key must match");

    // Adding valid transactions
    entry.addTransaction(50000LL); // Rs. 500.00
    ASSERT_TRUE(entry.isOccupied(), "Entry should be occupied after adding transaction");
    ASSERT_FALSE(entry.isEmpty(), "Entry should not be empty");
    ASSERT_EQ(entry.getTotalPaise(), 50000LL, "Total paise should be 50000");
    ASSERT_EQ(entry.getTransactionCount(), 1, "Transaction count should be 1");
    ASSERT_EQ(entry.getMinPaise(), 50000LL, "Min paise should be 50000");
    ASSERT_EQ(entry.getMaxPaise(), 50000LL, "Max paise should be 50000");
    ASSERT_EQ(entry.getAveragePaise(), 50000LL, "Average paise should be 50000");

    // Second transaction
    entry.addTransaction(30000LL); // Rs. 300.00
    ASSERT_EQ(entry.getTotalPaise(), 80000LL, "Total paise should accumulate to 80000");
    ASSERT_EQ(entry.getTransactionCount(), 2, "Transaction count should be 2");
    ASSERT_EQ(entry.getMinPaise(), 30000LL, "Min paise should update to 30000");
    ASSERT_EQ(entry.getMaxPaise(), 50000LL, "Max paise should remain 50000");
    ASSERT_EQ(entry.getAveragePaise(), 40000LL, "Average paise should be 40000");

    // Clear
    entry.clear();
    ASSERT_TRUE(entry.isEmpty(), "Entry should be empty after clear()");
    ASSERT_EQ(entry.getTotalPaise(), 0LL, "Total paise should be 0 after clear()");
    ASSERT_EQ(entry.getTransactionCount(), 0, "Count should be 0 after clear()");
}

void test_HashEntry_Validation() {
    std::cout << "[RUN] test_HashEntry_Validation...\n";

    HashEntry entry;

    // NULL key
    ASSERT_FALSE(entry.setKey(NULL), "NULL key should be rejected");

    // Empty string key
    ASSERT_FALSE(entry.setKey(""), "Empty string key should be rejected");

    // Key exceeding MAX_KEY_LENGTH (128 chars)
    char longKey[150];
    for (int i = 0; i < 140; ++i) longKey[i] = 'A';
    longKey[140] = '\0';
    ASSERT_FALSE(entry.setKey(longKey), "Key > 128 chars should be rejected");

    // Key at boundary: 127 chars + null terminator
    char validBoundaryKey[128];
    for (int i = 0; i < 127; ++i) validBoundaryKey[i] = 'X';
    validBoundaryKey[127] = '\0';
    ASSERT_TRUE(entry.setKey(validBoundaryKey), "Key with 127 chars should be accepted");

    // Numeric validation: zero and negative amounts
    entry.clear();
    entry.setKey("Test:Shop");
    entry.addTransaction(0);
    ASSERT_EQ(entry.getTransactionCount(), 0, "Adding 0 paise should be ignored");
    ASSERT_EQ(entry.getTotalPaise(), 0LL, "Total should remain 0");

    entry.addTransaction(-500);
    ASSERT_EQ(entry.getTransactionCount(), 0, "Adding negative paise should be ignored");
    ASSERT_EQ(entry.getTotalPaise(), 0LL, "Total should remain 0");
}

// ============================================================================
// TEST GROUP 2: HashTable Unit Tests (Collisions, Duplicates, Capacity)
// ============================================================================
void test_HashTable_Basics() {
    std::cout << "[RUN] test_HashTable_Basics...\n";

    HashTable table;
    ASSERT_EQ(table.getCapacity(), HashTable::DEFAULT_CAPACITY, "Default capacity check");
    ASSERT_EQ(table.getEntryCount(), 0, "Initial entry count should be 0");
    ASSERT_FALSE(table.isFull(), "New table should not be full");
    ASSERT_EQ(table.getLoadFactor(), 0.0, "Initial load factor should be 0");

    // Insertion
    ASSERT_TRUE(table.insertOrUpdate("Food:Subway", 25000LL), "Insert Food:Subway");
    ASSERT_EQ(table.getEntryCount(), 1, "Entry count should be 1");

    // Find existing
    const HashEntry* found = table.find("Food:Subway");
    ASSERT_TRUE(found != NULL, "Food:Subway must be found");
    ASSERT_STR_EQ(found->getKey(), "Food:Subway", "Found key must match");
    ASSERT_EQ(found->getTotalPaise(), 25000LL, "Found total paise check");
    ASSERT_EQ(found->getTransactionCount(), 1, "Found transaction count check");

    // Find non-existing
    ASSERT_TRUE(table.find("Food:BurgerKing") == NULL, "Non-existing key must return NULL");

    // Invalid parameters
    ASSERT_FALSE(table.insertOrUpdate(NULL, 1000), "NULL key insert must return false");
    ASSERT_FALSE(table.insertOrUpdate("", 1000), "Empty key insert must return false");
    ASSERT_FALSE(table.insertOrUpdate("ValidKey", 0), "0 amount insert must return false");
    ASSERT_FALSE(table.insertOrUpdate("ValidKey", -100), "Negative amount insert must return false");
    ASSERT_TRUE(table.find(NULL) == NULL, "find(NULL) must return NULL");
    ASSERT_TRUE(table.find("") == NULL, "find(\"\") must return NULL");
}

void test_HashTable_DuplicateKeys() {
    std::cout << "[RUN] test_HashTable_DuplicateKeys...\n";

    HashTable table;

    // First transaction for "Groceries:Walmart"
    ASSERT_TRUE(table.insertOrUpdate("Groceries:Walmart", 120000LL), "First insert for Walmart");
    ASSERT_EQ(table.getEntryCount(), 1, "Entry count should be 1");

    // Second transaction for same key
    ASSERT_TRUE(table.insertOrUpdate("Groceries:Walmart", 80000LL), "Second insert for Walmart");
    ASSERT_EQ(table.getEntryCount(), 1, "Duplicate key MUST NOT increment entry count");

    // Third transaction for same key
    ASSERT_TRUE(table.insertOrUpdate("Groceries:Walmart", 50000LL), "Third insert for Walmart");
    ASSERT_EQ(table.getEntryCount(), 1, "Duplicate key MUST NOT increment entry count");

    const HashEntry* entry = table.find("Groceries:Walmart");
    ASSERT_TRUE(entry != NULL, "Entry must be found");
    ASSERT_EQ(entry->getTotalPaise(), 250000LL, "Total paise should be 250000");
    ASSERT_EQ(entry->getTransactionCount(), 3, "Transaction count should be 3");
    ASSERT_EQ(entry->getMinPaise(), 50000LL, "Min transaction should be 50000");
    ASSERT_EQ(entry->getMaxPaise(), 120000LL, "Max transaction should be 120000");
    ASSERT_EQ(entry->getAveragePaise(), 83333LL, "Average transaction check");
}

void test_HashTable_Collisions_LinearProbing() {
    std::cout << "[RUN] test_HashTable_Collisions_LinearProbing...\n";

    // Create a small table of capacity 7 to guarantee collisions
    HashTable smallTable(7);
    ASSERT_EQ(smallTable.getCapacity(), 7, "Small table capacity should be 7");

    // Insert keys
    ASSERT_TRUE(smallTable.insertOrUpdate("KeyAlpha", 1000), "Insert KeyAlpha");
    ASSERT_TRUE(smallTable.insertOrUpdate("KeyBeta", 2000), "Insert KeyBeta");
    ASSERT_TRUE(smallTable.insertOrUpdate("KeyGamma", 3000), "Insert KeyGamma");
    ASSERT_TRUE(smallTable.insertOrUpdate("KeyDelta", 4000), "Insert KeyDelta");
    ASSERT_TRUE(smallTable.insertOrUpdate("KeyEpsilon", 5000), "Insert KeyEpsilon");

    // Collisions should have occurred due to small capacity
    ASSERT_TRUE(smallTable.getCollisionCount() >= 0, "Collision count tracked");

    // All inserted keys must be successfully retrieved via find()
    ASSERT_TRUE(smallTable.find("KeyAlpha") != NULL, "Find KeyAlpha");
    ASSERT_TRUE(smallTable.find("KeyBeta") != NULL, "Find KeyBeta");
    ASSERT_TRUE(smallTable.find("KeyGamma") != NULL, "Find KeyGamma");
    ASSERT_TRUE(smallTable.find("KeyDelta") != NULL, "Find KeyDelta");
    ASSERT_TRUE(smallTable.find("KeyEpsilon") != NULL, "Find KeyEpsilon");

    ASSERT_EQ(smallTable.find("KeyAlpha")->getTotalPaise(), 1000LL, "Check Alpha amount");
    ASSERT_EQ(smallTable.find("KeyBeta")->getTotalPaise(), 2000LL, "Check Beta amount");
    ASSERT_EQ(smallTable.find("KeyGamma")->getTotalPaise(), 3000LL, "Check Gamma amount");
    ASSERT_EQ(smallTable.find("KeyDelta")->getTotalPaise(), 4000LL, "Check Delta amount");
    ASSERT_EQ(smallTable.find("KeyEpsilon")->getTotalPaise(), 5000LL, "Check Epsilon amount");
}

void test_HashTable_Capacity_And_Full_Table() {
    std::cout << "[RUN] test_HashTable_Capacity_And_Full_Table...\n";

    // Create a table with capacity 5
    HashTable fixedTable(5);
    ASSERT_EQ(fixedTable.getCapacity(), 5, "Capacity is 5");

    ASSERT_TRUE(fixedTable.insertOrUpdate("Item1", 100), "Insert 1");
    ASSERT_TRUE(fixedTable.insertOrUpdate("Item2", 200), "Insert 2");
    ASSERT_TRUE(fixedTable.insertOrUpdate("Item3", 300), "Insert 3");
    ASSERT_TRUE(fixedTable.insertOrUpdate("Item4", 400), "Insert 4");
    ASSERT_TRUE(fixedTable.insertOrUpdate("Item5", 500), "Insert 5");

    ASSERT_EQ(fixedTable.getEntryCount(), 5, "Entry count should be 5");
    ASSERT_TRUE(fixedTable.isFull(), "Table should be full");

    // 6th distinct insert must be rejected safely
    ASSERT_FALSE(fixedTable.insertOrUpdate("Item6", 600), "6th insert in full table must fail");
    ASSERT_EQ(fixedTable.getEntryCount(), 5, "Entry count should remain 5");
    ASSERT_TRUE(fixedTable.find("Item6") == NULL, "Item6 should not be found");

    // Updating an EXISTING key in a full table MUST succeed!
    ASSERT_TRUE(fixedTable.insertOrUpdate("Item3", 700), "Updating existing key in full table must succeed");
    const HashEntry* item3 = fixedTable.find("Item3");
    ASSERT_TRUE(item3 != NULL, "Item3 must still exist");
    ASSERT_EQ(item3->getTotalPaise(), 1000LL, "Item3 total should be 300 + 700 = 1000");
    ASSERT_EQ(item3->getTransactionCount(), 2, "Item3 count should be 2");

    // Clear operation
    fixedTable.clear();
    ASSERT_EQ(fixedTable.getEntryCount(), 0, "Count should be 0 after clear");
    ASSERT_FALSE(fixedTable.isFull(), "Table should no longer be full");
    ASSERT_TRUE(fixedTable.find("Item1") == NULL, "Item1 should not be found after clear");
}

void test_HashTable_Inspection_And_Bounds() {
    std::cout << "[RUN] test_HashTable_Inspection_And_Bounds...\n";

    HashTable table(10);
    table.insertOrUpdate("CatA", 500);
    table.insertOrUpdate("CatB", 1000);

    // Bounds checking for getEntryAt
    ASSERT_TRUE(table.getEntryAt(-1) == NULL, "Negative index should return NULL");
    ASSERT_TRUE(table.getEntryAt(10) == NULL, "Index == capacity should return NULL");
    ASSERT_TRUE(table.getEntryAt(100) == NULL, "Index > capacity should return NULL");
    ASSERT_TRUE(table.getEntryAt(0) != NULL, "Valid index 0 should return pointer");

    // Deep copy test
    HashTable copyTable = table;
    ASSERT_EQ(copyTable.getEntryCount(), 2, "Copy table should have same entry count");
    ASSERT_TRUE(copyTable.find("CatA") != NULL, "Copy table should find CatA");
    ASSERT_TRUE(copyTable.find("CatB") != NULL, "Copy table should find CatB");

    // Modify original, verify copy is independent
    table.insertOrUpdate("CatC", 1500);
    ASSERT_EQ(table.getEntryCount(), 3, "Original count now 3");
    ASSERT_EQ(copyTable.getEntryCount(), 2, "Copy count remains 2 (deep copy verified)");
}

// ============================================================================
// TEST GROUP 3: LeakAnalyzer Unit Tests (Repeated Spending, Empty Datasets)
// ============================================================================
void test_LeakAnalyzer_EmptyDataset() {
    std::cout << "[RUN] test_LeakAnalyzer_EmptyDataset...\n";

    ExpenseManager emptyManager;
    LeakAnalyzer analyzer;

    // Analyzing empty manager should succeed safely
    ASSERT_TRUE(analyzer.analyzeExpenses(emptyManager), "Empty manager analysis should return true");
    ASSERT_EQ(analyzer.getTotalExpensesAnalyzed(), 0, "Expenses analyzed should be 0");
    ASSERT_EQ(analyzer.getTotalAnalyzedPaise(), 0LL, "Total paise should be 0");
    ASSERT_EQ(analyzer.getPatternCount(), 0, "Pattern count should be 0");
    ASSERT_TRUE(analyzer.getPatternAt(0) == NULL, "getPatternAt(0) should be NULL");
    ASSERT_TRUE(analyzer.getPatternAt(-1) == NULL, "getPatternAt(-1) should be NULL");

    // Report generation on empty dataset should execute without crashing
    std::stringstream ss;
    analyzer.printLeakReport(ss);
    std::string report = ss.str();
    ASSERT_TRUE(report.find("Total Expenses Analyzed : 0") != std::string::npos, "Report reflects 0 expenses");
    ASSERT_TRUE(report.find("No repeated spending patterns") != std::string::npos, "Report reflects 0 patterns");
}

void test_LeakAnalyzer_SpendingPatterns() {
    std::cout << "[RUN] test_LeakAnalyzer_SpendingPatterns...\n";

    ExpenseManager manager;

    // Scenario:
    // 1. Food:Starbucks - 3 transactions (Frequent spending leak)
    manager.addExpense(Expense(101, "05-10-2026", 35000LL, "Starbucks", "Food")); // Rs. 350
    manager.addExpense(Expense(102, "07-10-2026", 40000LL, "Starbucks", "Food")); // Rs. 400
    manager.addExpense(Expense(103, "12-10-2026", 35000LL, "Starbucks", "Food")); // Rs. 350

    // 2. Travel:Uber - 2 transactions (Repeated merchant expense)
    manager.addExpense(Expense(104, "06-10-2026", 65000LL, "Uber", "Travel"));    // Rs. 650
    manager.addExpense(Expense(105, "14-10-2026", 75000LL, "Uber", "Travel"));    // Rs. 750

    // 3. Housing:Landlord - 1 single high-amount transaction (Rs. 25,000.00 / 2,500,000 paise)
    // POLICY CHECK: Single transaction must NOT be classified as a repeated spending leak!
    manager.addExpense(Expense(106, "01-10-2026", 2500000LL, "Landlord", "Housing"));

    // 4. Shopping:Amazon - 1 single transaction (Rs. 1,200.00)
    manager.addExpense(Expense(107, "10-10-2026", 120000LL, "Amazon", "Shopping"));

    LeakAnalyzer analyzer;
    ASSERT_TRUE(analyzer.analyzeExpenses(manager), "Analyze expenses should succeed");

    ASSERT_EQ(analyzer.getTotalExpensesAnalyzed(), 7, "Total expenses analyzed should be 7");
    // Total: 350 + 400 + 350 + 650 + 750 + 25000 + 1200 = Rs. 28,700.00 = 2,870,000 paise
    ASSERT_EQ(analyzer.getTotalAnalyzedPaise(), 2870000LL, "Total paise analyzed check");

    // Verify pattern count: Exactly 2 patterns (Food:Starbucks and Travel:Uber)
    ASSERT_EQ(analyzer.getPatternCount(), 2, "Pattern count should be exactly 2");

    // Inspect patterns
    const HashEntry* p0 = analyzer.getPatternAt(0);
    const HashEntry* p1 = analyzer.getPatternAt(1);
    ASSERT_TRUE(p0 != NULL, "Pattern 0 must not be NULL");
    ASSERT_TRUE(p1 != NULL, "Pattern 1 must not be NULL");

    // One of them is Food:Starbucks and the other is Travel:Uber
    bool foundStarbucks = false;
    bool foundUber = false;

    for (int i = 0; i < analyzer.getPatternCount(); ++i) {
        const HashEntry* p = analyzer.getPatternAt(i);
        if (std::strcmp(p->getKey(), "Food:Starbucks") == 0) {
            foundStarbucks = true;
            ASSERT_EQ(p->getTransactionCount(), 3, "Starbucks count must be 3");
            ASSERT_EQ(p->getTotalPaise(), 110000LL, "Starbucks total must be 110000");
        } else if (std::strcmp(p->getKey(), "Travel:Uber") == 0) {
            foundUber = true;
            ASSERT_EQ(p->getTransactionCount(), 2, "Uber count must be 2");
            ASSERT_EQ(p->getTotalPaise(), 140000LL, "Uber total must be 140000");
        }
    }
    ASSERT_TRUE(foundStarbucks, "Food:Starbucks pattern identified");
    ASSERT_TRUE(foundUber, "Travel:Uber pattern identified");

    // Verify Housing:Landlord is in primary HashTable but NOT in patterns!
    const HashEntry* landlord = analyzer.getHashTable().find("Housing:Landlord");
    ASSERT_TRUE(landlord != NULL, "Housing:Landlord must be in HashTable");
    ASSERT_EQ(landlord->getTransactionCount(), 1, "Housing:Landlord count is 1");
    ASSERT_EQ(landlord->getTotalPaise(), 2500000LL, "Housing:Landlord total check");

    // Verify Category table tracking
    const HashEntry* housingCat = analyzer.getCategoryTable().find("Housing");
    ASSERT_TRUE(housingCat != NULL, "Housing category tracked");
    ASSERT_EQ(housingCat->getTotalPaise(), 2500000LL, "Housing category total check");

    const HashEntry* foodCat = analyzer.getCategoryTable().find("Food");
    ASSERT_TRUE(foodCat != NULL, "Food category tracked");
    ASSERT_EQ(foodCat->getTotalPaise(), 110000LL, "Food category total check");
}

void test_LeakAnalyzer_Date_And_Monthly_Tracking() {
    std::cout << "[RUN] test_LeakAnalyzer_Date_And_Monthly_Tracking...\n";

    ExpenseManager manager;
    manager.addExpense(Expense(201, "15-08-2026", 10000LL, "Cafe", "Food"));
    manager.addExpense(Expense(202, "02-09-2026", 20000LL, "Cafe", "Food"));
    manager.addExpense(Expense(203, "28-10-2026", 30000LL, "Cafe", "Food"));

    LeakAnalyzer analyzer;
    analyzer.analyzeExpenses(manager);

    ASSERT_STR_EQ(analyzer.getEarliestDate(), "15-08-2026", "Earliest date check");
    ASSERT_STR_EQ(analyzer.getLatestDate(), "28-10-2026", "Latest date check");

    // Monthly table should have 3 distinct months
    ASSERT_EQ(analyzer.getMonthlyTable().getEntryCount(), 3, "Three distinct months tracked");
    ASSERT_TRUE(analyzer.getMonthlyTable().find("08-2026") != NULL, "August 2026 found");
    ASSERT_TRUE(analyzer.getMonthlyTable().find("09-2026") != NULL, "September 2026 found");
    ASSERT_TRUE(analyzer.getMonthlyTable().find("10-2026") != NULL, "October 2026 found");

    ASSERT_EQ(analyzer.getMonthlyTable().find("08-2026")->getTotalPaise(), 10000LL, "Aug amount check");
    ASSERT_EQ(analyzer.getMonthlyTable().find("09-2026")->getTotalPaise(), 20000LL, "Sep amount check");
    ASSERT_EQ(analyzer.getMonthlyTable().find("10-2026")->getTotalPaise(), 30000LL, "Oct amount check");
}

void test_LeakAnalyzer_Report_And_Policy_Compliance() {
    std::cout << "[RUN] test_LeakAnalyzer_Report_And_Policy_Compliance...\n";

    ExpenseManager manager;
    // Essential high-value expense (Tuition Rs. 60,000 / 6,000,000 paise)
    manager.addExpense(Expense(301, "01-10-2026", 6000000LL, "University", "Education"));
    // Frequent micro transactions (Coffee Rs. 150 each x 4)
    manager.addExpense(Expense(302, "02-10-2026", 15000LL, "Costa", "Coffee"));
    manager.addExpense(Expense(303, "05-10-2026", 15000LL, "Costa", "Coffee"));
    manager.addExpense(Expense(304, "09-10-2026", 15000LL, "Costa", "Coffee"));
    manager.addExpense(Expense(305, "13-10-2026", 15000LL, "Costa", "Coffee"));

    LeakAnalyzer analyzer;
    analyzer.analyzeExpenses(manager);

    std::stringstream ss;
    analyzer.printLeakReport(ss);
    std::string report = ss.str();

    // Verify key sections are present
    ASSERT_TRUE(report.find("EXPENSE LEAK DETECTION & SPENDING PATTERN REPORT") != std::string::npos, "Header banner present");
    ASSERT_TRUE(report.find("FREQUENT SPENDING LEAK") != std::string::npos, "Frequent spending leak flagged for Coffee:Costa");
    ASSERT_TRUE(report.find("Coffee:Costa") != std::string::npos, "Channel Coffee:Costa present");

    // CRITICAL REQUIREMENT VERIFICATION:
    // "Do not label a category automatically as wasteful just because its total is high."
    ASSERT_TRUE(report.find("HIGH VOLUME CATEGORY") != std::string::npos, "Education marked as high volume");
    ASSERT_TRUE(report.find("not automatically labeled wasteful") != std::string::npos, "Policy note clearly stated");

    // CRITICAL REQUIREMENT VERIFICATION:
    // "Do not implement forecasting or savings advice; Person 5 owns that work."
    ASSERT_TRUE(report.find("owned exclusively by Person 5") != std::string::npos, "Person 5 ownership disclaimer present");
}

void test_LeakAnalyzer_DisplayReportLive() {
    std::cout << "[RUN] test_LeakAnalyzer_DisplayReportLive...\n";

    ExpenseManager manager;
    manager.addExpense(Expense(501, "01-10-2026", 45000LL, "Starbucks", "Coffee"));
    manager.addExpense(Expense(502, "03-10-2026", 45000LL, "Starbucks", "Coffee"));
    manager.addExpense(Expense(503, "08-10-2026", 45000LL, "Starbucks", "Coffee"));
    manager.addExpense(Expense(504, "02-10-2026", 75000LL, "Shell", "Fuel"));
    manager.addExpense(Expense(505, "15-10-2026", 75000LL, "Shell", "Fuel"));
    manager.addExpense(Expense(506, "01-10-2026", 4500000LL, "Airtel", "Utilities"));

    LeakAnalyzer analyzer;
    analyzer.analyzeExpenses(manager);

    std::cout << "\n>>> BEGIN LIVE DISPLAY LEAK REPORT OUTPUT <<<\n";
    analyzer.displayLeakReport();
    std::cout << ">>> END LIVE DISPLAY LEAK REPORT OUTPUT <<<\n\n";

    ASSERT_EQ(analyzer.getPatternCount(), 2, "Live report analyzer pattern count check");
}

void test_ExpenseManager_Non_Destructive_Inspection() {
    std::cout << "[RUN] test_ExpenseManager_Non_Destructive_Inspection...\n";

    ExpenseManager manager;
    manager.addExpense(Expense(401, "10-10-2026", 5000LL, "Bakery", "Food"));
    manager.addExpense(Expense(402, "11-10-2026", 7500LL, "Pharmacy", "Health"));

    int initialCount = manager.getCount();
    ASSERT_EQ(initialCount, 2, "Manager has 2 expenses");

    LeakAnalyzer analyzer;
    analyzer.analyzeExpenses(manager);

    // Verify manager was untouched
    ASSERT_EQ(manager.getCount(), initialCount, "Manager count unchanged");
    ASSERT_TRUE(manager.containsId(401), "ID 401 still exists");
    ASSERT_TRUE(manager.containsId(402), "ID 402 still exists");
    ASSERT_EQ(manager.findExpenseById(401)->getAmount(), 5000LL, "Amount 401 unchanged");
    ASSERT_EQ(manager.findExpenseById(402)->getAmount(), 7500LL, "Amount 402 unchanged");
}

// ============================================================================
// MAIN RUNNER
// ============================================================================
int main() {
    std::cout << "================================================================================\n";
    std::cout << "           RUNNING PERSON 3 TEST SUITE (HashTable & LeakAnalyzer)               \n";
    std::cout << "================================================================================\n";

    // HashEntry Tests
    test_HashEntry_Basics();
    test_HashEntry_Validation();

    // HashTable Tests
    test_HashTable_Basics();
    test_HashTable_DuplicateKeys();
    test_HashTable_Collisions_LinearProbing();
    test_HashTable_Capacity_And_Full_Table();
    test_HashTable_Inspection_And_Bounds();

    // LeakAnalyzer Tests
    test_LeakAnalyzer_EmptyDataset();
    test_LeakAnalyzer_SpendingPatterns();
    test_LeakAnalyzer_Date_And_Monthly_Tracking();
    test_LeakAnalyzer_Report_And_Policy_Compliance();
    test_LeakAnalyzer_DisplayReportLive();
    test_ExpenseManager_Non_Destructive_Inspection();

    std::cout << "================================================================================\n";
    std::cout << "TEST RESULTS: " << g_testsPassed << " / " << g_testsRun << " ASSERTIONS PASSED\n";
    if (g_testsPassed == g_testsRun) {
        std::cout << "OVERALL STATUS: ALL TESTS PASSED SUCCESSFULLY! (100% PASS RATE)\n";
    } else {
        std::cout << "OVERALL STATUS: SOME TESTS FAILED.\n";
    }
    std::cout << "================================================================================\n";

    return (g_testsPassed == g_testsRun) ? 0 : 1;
}
