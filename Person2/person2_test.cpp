/*
 * ============================================================================
 * PERSON 2 TEST SUITE
 * File: person2_test.cpp
 *
 * Comprehensive tests for:
 * - RecordParser (syntax, fields, conversions, dates, constraints)
 * - FileManager (load, save, missing, empty, malformed, duplicate, transactional)
 * - ReportExporter (monthly filtering, paise calculations, rupee formatting)
 *
 * NOTE: Strict compliance with No-STL container and algorithm constraints.
 * ============================================================================
 */

#include "RecordParser.h"
#include "FileManager.h"
#include "ReportExporter.h"
#include "Expense.h"
#include "ExpenseManager.h"

#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstring>

static int g_totalTests = 0;
static int g_passedTests = 0;
static int g_failedTests = 0;

void runAssert(bool condition, const char* testName, const char* details = NULL) {
    g_totalTests++;
    if (condition) {
        g_passedTests++;
        std::cout << "  [PASS] " << testName << "\n";
    } else {
        g_failedTests++;
        std::cout << "  [FAIL] " << testName;
        if (details != NULL && *details != '\0') {
            std::cout << " (" << details << ")";
        }
        std::cout << "\n";
    }
}

// Helper to remove test file safely
void removeFile(const char* path) {
    if (path != NULL) {
        std::remove(path);
    }
}

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#include <direct.h>
#define TEST_MKDIR(dir) _mkdir(dir)
#else
#include <sys/stat.h>
#define TEST_MKDIR(dir) mkdir(dir, 0755)
#endif

// Helper to write string content to a file
bool writeStringToFile(const char* path, const char* content) {
    if (path == NULL) return false;

    // Ensure parent directory exists
    const char* lastSlash = NULL;
    for (const char* p = path; *p != '\0'; ++p) {
        if (*p == '/' || *p == '\\') lastSlash = p;
    }
    if (lastSlash != NULL) {
        char dir[260];
        int len = (int)(lastSlash - path);
        std::strncpy(dir, path, len);
        dir[len] = '\0';
        TEST_MKDIR(dir);
    }

    std::ofstream out(path, std::ios::out | std::ios::trunc);
    if (!out.is_open()) return false;
    if (content != NULL) {
        out << content;
    }
    out.close();
    return true;
}

// ----------------------------------------------------------------------------
// Test Group 1: RecordParser
// ----------------------------------------------------------------------------
void testRecordParser() {
    std::cout << "\n=== Test Group 1: RecordParser ===\n";
    RecordParser parser;

    // 1. Valid ONE_TIME expense
    {
        Expense exp;
        const char* line = "104|15-10-2026|15000|Coffee Shop|Food|ONE_TIME|NONE";
        bool ok = parser.parseRecord(line, exp);
        runAssert(ok, "Parse valid ONE_TIME expense record");
        runAssert(exp.getId() == 104, "Verify ID == 104");
        runAssert(std::strcmp(exp.getDate(), "15-10-2026") == 0, "Verify Date == 15-10-2026");
        runAssert(exp.getAmount() == 15000, "Verify Amount == 15000 paise");
        runAssert(std::strcmp(exp.getMerchant(), "Coffee Shop") == 0, "Verify Merchant == Coffee Shop");
        runAssert(std::strcmp(exp.getCategory(), "Food") == 0, "Verify Category == Food");
        runAssert(exp.getType() == EXPENSE_ONE_TIME, "Verify Type == EXPENSE_ONE_TIME");
        runAssert(exp.getFrequency() == FREQ_NONE, "Verify Frequency == FREQ_NONE");
    }

    // 2. Valid RECURRING expense
    {
        Expense exp;
        const char* line = "105|15-10-2026|80000|Internet|Utilities|RECURRING|MONTHLY";
        bool ok = parser.parseRecord(line, exp);
        runAssert(ok, "Parse valid RECURRING expense record");
        runAssert(exp.getId() == 105, "Verify ID == 105");
        runAssert(exp.getAmount() == 80000, "Verify Amount == 80000 paise");
        runAssert(exp.getType() == EXPENSE_RECURRING, "Verify Type == EXPENSE_RECURRING");
        runAssert(exp.getFrequency() == FREQ_MONTHLY, "Verify Frequency == FREQ_MONTHLY");
    }

    // 3. Decimal currency conversion to paise
    {
        Expense exp;
        const char* line = "201|05-11-2026|150.75|Bakery|Food|ONE_TIME|NONE";
        bool ok = parser.parseRecord(line, exp);
        runAssert(ok, "Parse decimal currency '150.75'");
        runAssert(exp.getAmount() == 15075, "Verify decimal converted to 15075 paise");
    }

    // 4. Reject records with incorrect number of pipes
    {
        Expense exp;
        runAssert(!parser.parseRecord("104|15-10-2026|15000|Coffee Shop|Food|ONE_TIME", exp),
                  "Reject line with 5 pipes (too few fields)");
        runAssert(!parser.parseRecord("104|15-10-2026|15000|Coffee Shop|Food|ONE_TIME|NONE|EXTRA", exp),
                  "Reject line with 7 pipes (too many fields)");
    }

    // 5. Reject embedded pipe in fields
    {
        Expense exp;
        runAssert(!parser.parseRecord("104|15-10-2026|15000|Coffee|Shop|Food|ONE_TIME|NONE", exp),
                  "Reject field containing embedded '|'");
    }

    // 6. Reject invalid calendar dates
    {
        Expense exp;
        runAssert(!parser.parseRecord("104|31-02-2026|15000|Coffee Shop|Food|ONE_TIME|NONE", exp),
                  "Reject invalid leap/month date 31-02-2026");
        runAssert(!parser.parseRecord("104|15-13-2026|15000|Coffee Shop|Food|ONE_TIME|NONE", exp),
                  "Reject invalid month 15-13-2026");
        runAssert(!parser.parseRecord("104|00-10-2026|15000|Coffee Shop|Food|ONE_TIME|NONE", exp),
                  "Reject invalid day 00-10-2026");
        runAssert(!parser.parseRecord("104|15/10/2026|15000|Coffee Shop|Food|ONE_TIME|NONE", exp),
                  "Reject wrong delimiter 15/10/2026");

        // Leap year validation: 29-02-2024 is valid (leap), 29-02-2023 is invalid
        runAssert(parser.parseRecord("104|29-02-2024|15000|Coffee Shop|Food|ONE_TIME|NONE", exp),
                  "Accept 29-02-2024 (valid leap year)");
        runAssert(!parser.parseRecord("104|29-02-2023|15000|Coffee Shop|Food|ONE_TIME|NONE", exp),
                  "Reject 29-02-2023 (non-leap year)");
    }

    // 7. Reject invalid amounts
    {
        Expense exp;
        runAssert(!parser.parseRecord("104|15-10-2026|0|Coffee Shop|Food|ONE_TIME|NONE", exp),
                  "Reject zero amount");
        runAssert(!parser.parseRecord("104|15-10-2026|-500|Coffee Shop|Food|ONE_TIME|NONE", exp),
                  "Reject negative amount");
        runAssert(!parser.parseRecord("104|15-10-2026|abc|Coffee Shop|Food|ONE_TIME|NONE", exp),
                  "Reject non-numeric amount");
        runAssert(!parser.parseRecord("104|15-10-2026|150.|Coffee Shop|Food|ONE_TIME|NONE", exp),
                  "Reject trailing dot with no decimal digits");
        runAssert(!parser.parseRecord("104|15-10-2026|.50|Coffee Shop|Food|ONE_TIME|NONE", exp),
                  "Reject leading dot without integer portion");
        runAssert(!parser.parseRecord("104|15-10-2026|150.123|Coffee Shop|Food|ONE_TIME|NONE", exp),
                  "Reject more than 2 decimal places");
        runAssert(!parser.parseRecord("104|15-10-2026|150.10.20|Coffee Shop|Food|ONE_TIME|NONE", exp),
                  "Reject multiple decimal points");
    }

    // 8. Reject invalid types and frequencies
    {
        Expense exp;
        runAssert(!parser.parseRecord("104|15-10-2026|15000|Coffee Shop|Food|INVALID_TYPE|NONE", exp),
                  "Reject unknown type");
        runAssert(!parser.parseRecord("104|15-10-2026|15000|Coffee Shop|Food|ONE_TIME|MONTHLY", exp),
                  "Reject ONE_TIME with recurring frequency MONTHLY");
        runAssert(!parser.parseRecord("104|15-10-2026|15000|Coffee Shop|Food|RECURRING|NONE", exp),
                  "Reject RECURRING with frequency NONE");
    }

    // 9. Reject empty / all-whitespace fields
    {
        Expense exp;
        runAssert(!parser.parseRecord("104|15-10-2026|15000||Food|ONE_TIME|NONE", exp),
                  "Reject empty merchant");
        runAssert(!parser.parseRecord("104|15-10-2026|15000|Coffee Shop||ONE_TIME|NONE", exp),
                  "Reject empty category");
        runAssert(!parser.parseRecord("104|15-10-2026|15000|   |Food|ONE_TIME|NONE", exp),
                  "Reject all-whitespace merchant");
    }

    // 10. Direct validateRecord validation
    {
        Expense validExp(301, "20-10-2026", 5000, "Stationery", "Office", EXPENSE_ONE_TIME, FREQ_NONE);
        runAssert(parser.validateRecord(validExp), "validateRecord() returns true for valid object");

        Expense invalidExp(0, "20-10-2026", 5000, "Stationery", "Office", EXPENSE_ONE_TIME, FREQ_NONE);
        runAssert(!parser.validateRecord(invalidExp), "validateRecord() rejects ID <= 0");
    }
}

// ----------------------------------------------------------------------------
// Test Group 2: FileManager
// ----------------------------------------------------------------------------
void testFileManager() {
    std::cout << "\n=== Test Group 2: FileManager ===\n";

    // 1. Missing file handling
    {
        const char* missingPath = "test_data/non_existent_file_12345.txt";
        removeFile(missingPath);

        FileManager fm(missingPath);
        ExpenseManager manager;
        // Pre-populate manager with valid record
        Expense preExp(999, "01-01-2026", 1000, "Init", "Test");
        manager.addLoadedExpense(preExp);

        bool loadResult = fm.loadExpenses(manager);
        runAssert(!loadResult, "loadExpenses returns false for missing file");
        runAssert(std::strstr(fm.getLastError(), "Failed to open") != NULL,
                  "getLastError contains descriptive missing file message");
        runAssert(manager.getCount() == 1, "Existing in-memory data NOT erased on missing file");
    }

    // 2. Empty file handling
    {
        const char* emptyPath = "test_data/empty_expenses.txt";
        writeStringToFile(emptyPath, "");

        FileManager fm(emptyPath);
        ExpenseManager manager;
        // Pre-populate manager
        Expense preExp(999, "01-01-2026", 1000, "Init", "Test");
        manager.addLoadedExpense(preExp);

        bool loadResult = fm.loadExpenses(manager);
        runAssert(loadResult, "loadExpenses handles empty file safely and succeeds");
        runAssert(manager.getCount() == 0, "Manager count is 0 after loading empty file");

        removeFile(emptyPath);
    }

    // 3. Malformed records & line number reporting
    {
        const char* malformedPath = "test_data/malformed.txt";
        const char* content =
            "101|10-10-2026|1000|Item 1|Cat A|ONE_TIME|NONE\n"
            "102|99-99-9999|2000|Item 2|Cat B|ONE_TIME|NONE\n" // Line 2 malformed date!
            "103|12-10-2026|3000|Item 3|Cat C|ONE_TIME|NONE\n";
        writeStringToFile(malformedPath, content);

        FileManager fm(malformedPath);
        ExpenseManager manager;
        Expense initialExp(555, "01-01-2026", 500, "Original", "Safe");
        manager.addLoadedExpense(initialExp);

        bool loadResult = fm.loadExpenses(manager);
        runAssert(!loadResult, "loadExpenses returns false on malformed file");
        runAssert(std::strstr(fm.getLastError(), "Line 2") != NULL,
                  "Error accurately reports malformed Line 2");
        runAssert(manager.getCount() == 1, "Transactional safety: manager was NOT corrupted or wiped");
        runAssert(manager.findExpenseById(555) != NULL, "Original record 555 preserved intact");

        removeFile(malformedPath);
    }

    // 4. Duplicate IDs & line number reporting
    {
        const char* dupPath = "test_data/duplicate_id.txt";
        const char* content =
            "101|10-10-2026|1000|Item 1|Cat A|ONE_TIME|NONE\n"
            "102|11-10-2026|2000|Item 2|Cat B|ONE_TIME|NONE\n"
            "101|12-10-2026|3000|Item 3|Cat C|ONE_TIME|NONE\n"; // Line 3 has duplicate ID 101!
        writeStringToFile(dupPath, content);

        FileManager fm(dupPath);
        ExpenseManager manager;
        bool loadResult = fm.loadExpenses(manager);
        runAssert(!loadResult, "loadExpenses rejects duplicate ID in file");
        runAssert(std::strstr(fm.getLastError(), "Line 3") != NULL,
                  "Error accurately reports Line 3 for duplicate");
        runAssert(std::strstr(fm.getLastError(), "101") != NULL,
                  "Error explicitly mentions duplicate ID 101");
        runAssert(manager.getCount() == 0, "Manager remains clean after rejected duplicate load");

        removeFile(dupPath);
    }

    // 5. Save and reload round-trip
    {
        const char* testSavePath = "test_data/roundtrip_expenses.txt";
        removeFile(testSavePath);

        ExpenseManager srcManager;
        Expense e1(104, "15-10-2026", 15000, "Coffee Shop", "Food", EXPENSE_ONE_TIME, FREQ_NONE);
        Expense e2(105, "15-10-2026", 80000, "Internet", "Utilities", EXPENSE_RECURRING, FREQ_MONTHLY);
        Expense e3(106, "16-10-2026", 4550, "Metro Ride", "Transport", EXPENSE_ONE_TIME, FREQ_NONE);

        srcManager.addLoadedExpense(e1);
        srcManager.addLoadedExpense(e2);
        srcManager.addLoadedExpense(e3);

        FileManager fm(testSavePath);
        bool saveOk = fm.saveExpenses(srcManager);
        runAssert(saveOk, "saveExpenses writes snapshot and saves successfully");

        // Reload into a separate ExpenseManager
        ExpenseManager loadedManager;
        bool loadOk = fm.loadExpenses(loadedManager);
        runAssert(loadOk, "loadExpenses successfully reloads saved file");
        runAssert(loadedManager.getCount() == 3, "Reloaded manager has exactly 3 records");

        Expense* re1 = loadedManager.findExpenseById(104);
        Expense* re2 = loadedManager.findExpenseById(105);
        Expense* re3 = loadedManager.findExpenseById(106);

        runAssert(re1 != NULL && re1->getAmount() == 15000 && std::strcmp(re1->getMerchant(), "Coffee Shop") == 0,
                  "Preserved ID 104, amount 15000, merchant 'Coffee Shop'");
        runAssert(re2 != NULL && re2->getAmount() == 80000 && re2->getType() == EXPENSE_RECURRING && re2->getFrequency() == FREQ_MONTHLY,
                  "Preserved ID 105, RECURRING, MONTHLY frequency");
        runAssert(re3 != NULL && re3->getAmount() == 4550 && std::strcmp(re3->getCategory(), "Transport") == 0,
                  "Preserved ID 106, amount 4550, category 'Transport'");

        removeFile(testSavePath);
    }

    // 6. Write failure handling
    {
        // Invalid filename containing illegal characters on Windows
        const char* invalidPath = "test_data/invalid<>:\"|?*file.txt";
        FileManager fm(invalidPath);
        ExpenseManager mgr;
        Expense exp(1, "10-10-2026", 100, "Test", "Test");
        mgr.addLoadedExpense(exp);

        bool saveOk = fm.saveExpenses(mgr);
        runAssert(!saveOk, "saveExpenses returns false on write failure/invalid path");
        runAssert(fm.getLastError()[0] != '\0', "getLastError reports accurate write failure reason");
    }
}

// ----------------------------------------------------------------------------
// Test Group 3: ReportExporter
// ----------------------------------------------------------------------------
void testReportExporter() {
    std::cout << "\n=== Test Group 3: ReportExporter ===\n";

    ReportExporter exporter;
    ExpenseManager manager;

    // Set up dataset across multiple months and years
    // October 2026 (Month 10, Year 2026)
    Expense e1(101, "05-10-2026", 5000, "Bakery", "Food", EXPENSE_ONE_TIME, FREQ_NONE);            // Rs. 50.00
    Expense e2(104, "15-10-2026", 15000, "Coffee Shop", "Food", EXPENSE_ONE_TIME, FREQ_NONE);       // Rs. 150.00
    Expense e3(105, "18-10-2026", 80000, "Internet", "Utilities", EXPENSE_RECURRING, FREQ_MONTHLY); // Rs. 800.00
    Expense e4(106, "22-10-2026", 4550, "Metro Card", "Transport", EXPENSE_ONE_TIME, FREQ_NONE);    // Rs. 45.50
    // Total Oct 2026: 4 transactions, 104550 paise = Rs. 1045.50

    // November 2026 (Month 11, Year 2026)
    Expense e5(201, "02-11-2026", 12000, "Grocery Mart", "Food", EXPENSE_ONE_TIME, FREQ_NONE);      // Rs. 120.00

    // October 2025 (Month 10, Year 2025 - different year!)
    Expense e6(301, "15-10-2025", 9999, "Old Store", "Shopping", EXPENSE_ONE_TIME, FREQ_NONE);      // Rs. 99.99

    manager.addLoadedExpense(e1);
    manager.addLoadedExpense(e2);
    manager.addLoadedExpense(e3);
    manager.addLoadedExpense(e4);
    manager.addLoadedExpense(e5);
    manager.addLoadedExpense(e6);

    // 1. Export October 2026 report
    {
        const char* octReportPath = "reports/monthly_oct_2026.txt";
        removeFile(octReportPath);

        bool ok = exporter.exportMonthlyReport(manager, 10, 2026, octReportPath);
        runAssert(ok, "exportMonthlyReport succeeds for October 2026");

        // Verify report file contents
        std::ifstream file(octReportPath);
        runAssert(file.is_open(), "Generated report file exists and opened");

        char line[512];
        bool foundPeriod = false;
        bool foundCount4 = false;
        bool foundTotal1045 = false;
        bool foundCoffee = false;
        bool foundOldStore = false; // Must NOT be in Oct 2026 report

        while (file.getline(line, sizeof(line))) {
            if (std::strstr(line, "October 2026") != NULL) foundPeriod = true;
            if (std::strstr(line, "Total Transactions: 4") != NULL ||
                std::strstr(line, "Transactions Filtered : 4") != NULL) foundCount4 = true;
            if (std::strstr(line, "1045.50") != NULL) foundTotal1045 = true;
            if (std::strstr(line, "Coffee Shop") != NULL) foundCoffee = true;
            if (std::strstr(line, "Old Store") != NULL) foundOldStore = true;
        }
        file.close();

        runAssert(foundPeriod, "Report header contains 'October 2026'");
        runAssert(foundCount4, "Report identifies exactly 4 matching transactions");
        runAssert(foundTotal1045, "Report calculates total spending as Rs. 1045.50");
        runAssert(foundCoffee, "October transaction 'Coffee Shop' included in report table");
        runAssert(!foundOldStore, "October 2025 transaction 'Old Store' correctly excluded");

        removeFile(octReportPath);
    }

    // 2. Export month with 0 transactions
    {
        const char* zeroReportPath = "reports/monthly_aug_2026.txt";
        removeFile(zeroReportPath);

        bool ok = exporter.exportMonthlyReport(manager, 8, 2026, zeroReportPath);
        runAssert(ok, "exportMonthlyReport succeeds for month with 0 transactions");

        std::ifstream file(zeroReportPath);
        runAssert(file.is_open(), "Empty month report file exists and opened");

        char line[512];
        bool foundCount0 = false;
        bool foundTotalZero = false;

        while (file.getline(line, sizeof(line))) {
            if (std::strstr(line, "Total Transactions: 0") != NULL ||
                std::strstr(line, "Transactions Filtered : 0") != NULL) foundCount0 = true;
            if (std::strstr(line, "0.00") != NULL) foundTotalZero = true;
        }
        file.close();

        runAssert(foundCount0, "Report identifies 0 matching transactions");
        runAssert(foundTotalZero, "Report displays total spending as Rs. 0.00");

        removeFile(zeroReportPath);
    }

    // 3. Invalid parameters validation
    {
        runAssert(!exporter.exportMonthlyReport(manager, 0, 2026, "reports/bad.txt"),
                  "Reject invalid month 0");
        runAssert(!exporter.exportMonthlyReport(manager, 13, 2026, "reports/bad.txt"),
                  "Reject invalid month 13");
        runAssert(!exporter.exportMonthlyReport(manager, 10, 1850, "reports/bad.txt"),
                  "Reject invalid year 1850");
        runAssert(!exporter.exportMonthlyReport(manager, 10, 2026, NULL),
                  "Reject NULL output path");
        runAssert(!exporter.exportMonthlyReport(manager, 10, 2026, ""),
                  "Reject empty output path");
    }
}

// ----------------------------------------------------------------------------
// Main Test Runner
// ----------------------------------------------------------------------------
int main() {
    std::cout << "=========================================================\n";
    std::cout << "          PERSON 2 MODULE VERIFICATION TEST SUITE        \n";
    std::cout << "=========================================================\n";

    testRecordParser();
    testFileManager();
    testReportExporter();

    std::cout << "\n=========================================================\n";
    std::cout << "                  TEST EXECUTION SUMMARY                 \n";
    std::cout << "=========================================================\n";
    std::cout << " Total Test Assertions: " << g_totalTests << "\n";
    std::cout << " Passed:                " << g_passedTests << "\n";
    std::cout << " Failed:                " << g_failedTests << "\n";

    if (g_failedTests == 0) {
        std::cout << "\n >>> ALL PERSON 2 TESTS PASSED SUCCESSFULLY! <<<\n";
    } else {
        std::cout << "\n >>> WARNING: SOME TESTS FAILED! <<<\n";
    }
    std::cout << "=========================================================\n";

    return (g_failedTests == 0) ? 0 : 1;
}
