/*
 * Integration smoke tests for interfaces currently present in the supplied
 * bundles. This test intentionally avoids assuming the missing Person 2 API.
 * It has its own main(); compile it separately from main.cpp.
 */
#include <iostream>
#include <fstream>
#include <cstdio>
#include <climits>
#include <cstring>
#include <sstream>
#include <string>
#include "Expense.h"
#include "ExpenseManager.h"
#include "FileManager.h"
#include "RecordParser.h"
#include "ReportExporter.h"
#include "LeakAnalyzer.h"
#include "Search.h"
#include "Sort.h"
#include "ExpenseBST.h"
#include "SavingsAdvisor.h"
#include "UndoStack.h"
#include "UndoAction.h"
#include "UI.h"
#include "GuiLogic.h"

static int passed = 0;
static int failed = 0;
#define CHECK(expr, label) do { \
    if (expr) { ++passed; std::cout << "[PASS] " << label << "\n"; } \
    else { ++failed; std::cout << "[FAIL] " << label << "\n"; } \
} while (0)

int main() {
    std::cout << "Expense Leak Detector integration smoke tests\n";
    ExpenseManager manager;
    int id1 = 0, id2 = 0;
    CHECK(manager.addExpense("01-10-2026", 12500, "Cafe", "Food",
                             "ONE_TIME", "NONE", id1), "add first expense");
    CHECK(manager.addExpense("02-10-2026", 25000, "Bus", "Transport",
                             "ONE_TIME", "NONE", id2), "add second expense");
    CHECK(id1 > 0 && id2 > id1, "IDs assigned in increasing order");
    CHECK(manager.getCount() == 2, "manager count after add");

    Expense** pointers = new Expense*[manager.getCount()];
    int copied = manager.copyExpensePointers(pointers, manager.getCount());
    CHECK(copied == 2, "copy non-owning expense pointers");
    Search search;
    Sort sorter;
    CHECK(search.linearSearchById(pointers, copied, id1) != 0, "linear search finds existing ID");
    sorter.sortById(pointers, copied);
    CHECK(pointers[0]->getId() < pointers[1]->getId(), "sort by ID");
    ExpenseBST bst;
    CHECK(bst.insert(*pointers[0]) && bst.insert(*pointers[1]), "BST index inserts expenses");
    CHECK(bst.getCount() == 2, "BST count");
    delete[] pointers;

    const Expense* original = manager.findExpenseById(id1);
    Expense before(*original);
    Expense deletedSnapshot(*manager.findExpenseById(id2));
    Expense changed(id1, "03-10-2026", 15000, "Cafe Updated", "Food",
                    "ONE_TIME", "NONE");
    CHECK(manager.editExpense(id1, changed), "edit expense");
    CHECK(manager.findExpenseById(id1)->getAmountPaise() == 15000, "edited amount visible");
    CHECK(manager.deleteExpense(id2), "delete expense");
    CHECK(manager.getCount() == 1, "count after delete");
    CHECK(manager.addLoadedExpense(deletedSnapshot), "restore snapshot with preserved ID");
    CHECK(manager.findExpenseById(id2) != 0, "restored record retains ID");
    SavingsAdvisor advisor;
    CHECK(advisor.calculateMonthlyTotal(manager, 10, 2026) >= 0, "monthly total callable");
    long long parsedPaise = 0;
    CHECK(GuiLogic::parseRupees("123.45", parsedPaise) && parsedPaise == 12345,
          "GUI parses rupees into exact paise");
    CHECK(GuiLogic::parseRupees("92233720368547758.07", parsedPaise) &&
          parsedPaise == LLONG_MAX,
          "GUI accepts the largest representable paise amount");
    CHECK(!GuiLogic::parseRupees("1.001", parsedPaise) &&
          !GuiLogic::parseRupees("0", parsedPaise) &&
          !GuiLogic::parseRupees("92233720368547758.08", parsedPaise),
          "GUI rejects invalid and overflowing amounts");
    CategoryTotal categoryTotals[4];
    const int categoryCount =
        GuiLogic::buildCategoryTotals(manager, categoryTotals, 4);
    CHECK(categoryCount == 2 &&
          std::strcmp(categoryTotals[0].name, "Transport") == 0 &&
          categoryTotals[0].paise == 25000,
          "GUI category totals are ordered by spending");
    ExpenseManager largeManager;
    Expense hugeExpense(1, "10-10-2020", LLONG_MAX, "Large", "Other",
                        "ONE_TIME", "NONE");
    CHECK(largeManager.addLoadedExpense(hugeExpense), "accept maximum paise amount");
    CHECK(advisor.calculatePotentialSavings(largeManager, 10, 2020, 100) == LLONG_MAX,
          "100 percent savings avoids integer overflow");
    UndoStack stack;
    UndoAction action(ActionType::EDIT, id1, before, changed);
    CHECK(stack.push(action), "undo stack push");
    UndoAction popped;
    CHECK(stack.pop(popped) && popped.getExpenseId() == id1, "undo stack pop");

    RecordParser parser;
    Expense parsed;
    CHECK(parser.parseRecord("2147483647|01-01-0001|1|Shop|Other|ONE_TIME|NONE",
                             parsed),
          "parser accepts valid full-range ID and date");
    CHECK(!parser.parseRecord("2147483648|01-01-2026|1|Shop|Other|ONE_TIME|NONE",
                              parsed),
          "parser rejects overflowing ID");
    Expense invalidDelimiter(10, "01-01-2026", 100, "Shop|Name", "Other",
                             "ONE_TIME", "NONE");
    CHECK(!invalidDelimiter.isValid(), "reject pipe delimiters in text fields");

    FileManager fileManager("integration_test_expenses.txt");
    CHECK(fileManager.saveExpenses(manager), "save pipe-delimited snapshot");
    ExpenseManager loadedManager;
    CHECK(fileManager.loadExpenses(loadedManager), "load saved snapshot");
    CHECK(loadedManager.getCount() == manager.getCount(), "loaded record count");
    const Expense* loaded = loadedManager.findExpenseById(id1);
    CHECK(loaded && loaded->getAmountPaise() == 15000, "loaded values and IDs preserved");

    LeakAnalyzer analyzer;
    CHECK(analyzer.analyzeExpenses(loadedManager), "analyze loaded expenses");
    CHECK(analyzer.getTotalExpensesAnalyzed() == 2 &&
          analyzer.getTotalAnalyzedPaise() == 40000, "leak analyzer totals");
    std::ostringstream leakReport;
    analyzer.printLeakReport(leakReport);
    CHECK(leakReport.str().find(".00") == std::string::npos &&
          leakReport.str().find("(40000 paise)") == std::string::npos,
          "leak report omits unnecessary currency and paise zeroes");

    ReportExporter exporter;
    CHECK(exporter.exportMonthlyReport(loadedManager, 10, 2026,
                                      "integration_test_report.txt"),
          "export monthly report");
    std::ifstream report("integration_test_report.txt");
    CHECK(report.good(), "monthly report file created");
    std::string reportContents;
    std::string reportLine;
    while (std::getline(report, reportLine)) reportContents += reportLine + "\n";
    report.close();
    CHECK(reportContents.find("Transactions: 2") != std::string::npos &&
          reportContents.find("Total spending: Rs. 400") != std::string::npos &&
          reportContents.find(".00") == std::string::npos,
          "monthly report is readable and omits unnecessary decimal zeroes");
    char guiReportPath[260];
    char guiReportError[200];
    CHECK(GuiLogic::exportMonthlyReport(loadedManager, advisor,
                                        "integration_test_expenses.txt",
                                        10, 2026, guiReportPath,
                                        sizeof(guiReportPath), guiReportError,
                                        sizeof(guiReportError)),
          "GUI exports the shared monthly report with analysis");
    std::ifstream guiReport(guiReportPath);
    std::string guiReportContents;
    while (std::getline(guiReport, reportLine))
        guiReportContents += reportLine + "\n";
    guiReport.close();
    CHECK(guiReportContents.find("SPENDING LEAK ANALYSIS") != std::string::npos &&
          guiReportContents.find("FORECAST") != std::string::npos &&
          guiReportContents.find("Monthly total: Rs. 400") != std::string::npos,
          "GUI report includes monthly leak analysis and forecast");

    ExpenseManager centsManager;
    Expense centsOne(1, "01-10-2026", 12340, "Shop One", "Other",
                     "ONE_TIME", "NONE");
    Expense centsTwo(2, "02-10-2026", 12305, "Shop Two", "Other",
                     "ONE_TIME", "NONE");
    CHECK(centsManager.addLoadedExpense(centsOne) &&
          centsManager.addLoadedExpense(centsTwo), "prepare paise-format report");
    CHECK(exporter.exportMonthlyReport(centsManager, 10, 2026,
                                      "integration_test_cents_report.txt"),
          "export report with fractional rupees");
    std::ifstream centsReport("integration_test_cents_report.txt");
    std::string centsContents;
    while (std::getline(centsReport, reportLine)) centsContents += reportLine + "\n";
    centsReport.close();
    CHECK(centsContents.find("Rs. 123.4") != std::string::npos &&
          centsContents.find("Rs. 123.05") != std::string::npos &&
          centsContents.find("Rs. 123.40") == std::string::npos,
          "report trims trailing paise zero but preserves value");

    Expense outsideMonth(77, "01-11-2026", 99900, "Outside Month",
                         "Other", "ONE_TIME", "NONE");
    CHECK(loadedManager.addLoadedExpense(outsideMonth),
          "add expense outside requested report month");
    FileManager uiFileManager("integration_test_ui_expenses.txt");
    UI ui(loadedManager, uiFileManager, analyzer, search, sorter, bst, advisor, stack);
    std::istringstream uiInput("5\n1\n6\n3\n6\n4\n10\n10\n2026\n11\n");
    std::ostringstream uiOutput;
    std::streambuf* originalInput = std::cin.rdbuf(uiInput.rdbuf());
    std::streambuf* originalOutput = std::cout.rdbuf(uiOutput.rdbuf());
    ui.run();
    std::cin.rdbuf(originalInput);
    std::cout.rdbuf(originalOutput);

    CHECK(uiOutput.str().find("SEARCH RESULT") != std::string::npos &&
          uiOutput.str().find("Category           | Merchant") != std::string::npos &&
          uiOutput.str().find("Cafe Updated") != std::string::npos &&
          uiOutput.str().find("Rs. 150") != std::string::npos,
          "search result presents expense fields in an aligned table");
    const std::size_t dateSortTitle = uiOutput.str().find("EXPENSES SORTED BY DATE");
    const std::size_t transportCategory =
        uiOutput.str().find("CATEGORY: Transport", dateSortTitle);
    const std::size_t foodCategory =
        uiOutput.str().find("CATEGORY: Food", transportCategory);
    const std::size_t merchantHeader =
        uiOutput.str().find("Merchant", transportCategory);
    const std::size_t dateSortedSecond =
        uiOutput.str().find("2      | 02-10-2026", transportCategory);
    const std::size_t dateSortedFirst =
        uiOutput.str().find("1      | 03-10-2026", foodCategory);
    CHECK(dateSortTitle != std::string::npos &&
          transportCategory != std::string::npos &&
          foodCategory != std::string::npos &&
          merchantHeader != std::string::npos &&
          transportCategory < foodCategory &&
          dateSortedSecond != std::string::npos &&
          dateSortedFirst != std::string::npos &&
          dateSortedSecond < dateSortedFirst,
          "sorted rows group under category headings with merchants below and retain order");
    const std::size_t amountIndexTitle =
        uiOutput.str().find("EXPENSES SORTED BY AMOUNT INDEX");
    CHECK(amountIndexTitle != std::string::npos &&
          uiOutput.str().find("CATEGORY: Food", amountIndexTitle) != std::string::npos &&
          uiOutput.str().find("Merchant", amountIndexTitle) != std::string::npos,
          "amount index results use category groups and merchant rows");
    std::ifstream integratedReport("monthly_report_10-2026.txt");
    std::string integratedContents;
    while (std::getline(integratedReport, reportLine))
        integratedContents += reportLine + "\n";
    integratedReport.close();
    CHECK(integratedContents.find("SPENDING LEAK REPORT - 10/2026") != std::string::npos &&
          integratedContents.find("MONTHLY FORECAST") != std::string::npos,
          "UI export includes monthly leak report and forecast sections");
    CHECK(integratedContents.find("Actual spending: Rs. 400") != std::string::npos &&
          integratedContents.find("Projected month-end spending: Rs. 400") != std::string::npos,
          "monthly forecast uses the selected month's transactions");
    CHECK(integratedContents.find("Outside Month") == std::string::npos,
          "monthly report excludes expenses from other months");

    {
        std::ofstream invalidFile("integration_test_invalid.txt");
        invalidFile << "2|01-10-2026|100|Shop|Other|ONE_TIME|NONE\n";
        invalidFile << "not a valid expense record\n";
    }
    ExpenseManager preservedManager;
    Expense preserved(9, "05-10-2026", 500, "Keep", "Other",
                      "ONE_TIME", "NONE");
    CHECK(preservedManager.addLoadedExpense(preserved), "prepare existing data for failed load");
    FileManager invalidFileManager("integration_test_invalid.txt");
    CHECK(!invalidFileManager.loadExpenses(preservedManager),
          "reject malformed file without loading partial rows");
    CHECK(preservedManager.getCount() == 1 &&
          preservedManager.findExpenseById(9) != 0,
          "failed load leaves existing data untouched");

    std::remove("integration_test_expenses.txt");
    std::remove("integration_test_expenses.txt.tmp");
    std::remove("integration_test_report.txt");
    std::remove("integration_test_cents_report.txt");
    std::remove("monthly_report_10-2026.txt");
    std::remove("integration_test_invalid.txt");

    manager.clearExpenses();
    std::cout << "Passed: " << passed << " | Failed: " << failed << "\n";
    return failed == 0 ? 0 : 1;
}
