#include "GuiLogic.h"

#include "Expense.h"
#include "ExpenseNode.h"
#include "LeakAnalyzer.h"
#include "ReportExporter.h"
#include "SavingsAdvisor.h"

#include <cctype>
#include <climits>
#include <cstdio>
#include <cstring>
#include <fstream>

namespace {

void setError(char* error, int capacity, const char* message) {
    if (error == NULL || capacity <= 0) return;
    std::snprintf(error, static_cast<std::size_t>(capacity), "%s", message);
}

bool matchesMonthYear(const char* date, int month, int year) {
    if (date == NULL || std::strlen(date) != 10 ||
        date[2] != '-' || date[5] != '-') return false;
    for (int i = 0; i < 10; ++i) {
        if (i == 2 || i == 5) continue;
        if (date[i] < '0' || date[i] > '9') return false;
    }
    const int dateMonth = (date[3] - '0') * 10 + date[4] - '0';
    const int dateYear = (date[6] - '0') * 1000 + (date[7] - '0') * 100 +
                         (date[8] - '0') * 10 + date[9] - '0';
    return dateMonth == month && dateYear == year;
}

} // namespace

namespace GuiLogic {

bool containsIgnoreCase(const char* text, const char* query) {
    if (text == NULL || query == NULL) return false;
    if (*query == '\0') return true;

    for (const char* start = text; *start != '\0'; ++start) {
        const char* left = start;
        const char* right = query;
        while (*left != '\0' && *right != '\0' &&
               std::tolower(static_cast<unsigned char>(*left)) ==
                   std::tolower(static_cast<unsigned char>(*right))) {
            ++left;
            ++right;
        }
        if (*right == '\0') return true;
    }
    return false;
}

void trim(char* text) {
    if (text == NULL) return;
    char* first = text;
    while (*first != '\0' && std::isspace(static_cast<unsigned char>(*first)))
        ++first;

    char* end = first + std::strlen(first);
    while (end > first && std::isspace(static_cast<unsigned char>(end[-1])))
        --end;

    const std::size_t length = static_cast<std::size_t>(end - first);
    if (first != text) std::memmove(text, first, length);
    text[length] = '\0';
}

bool parseRupees(const char* text, long long& paise) {
    if (text == NULL || *text == '\0') return false;

    const unsigned long long maxWhole =
        static_cast<unsigned long long>(LLONG_MAX / 100);
    unsigned long long whole = 0;
    const char* cursor = text;
    bool hasWholeDigit = false;
    while (*cursor >= '0' && *cursor <= '9') {
        hasWholeDigit = true;
        const unsigned int digit = static_cast<unsigned int>(*cursor - '0');
        if (whole > (maxWhole - digit) / 10) return false;
        whole = whole * 10 + digit;
        ++cursor;
    }

    int cents = 0;
    if (*cursor == '.') {
        ++cursor;
        if (*cursor < '0' || *cursor > '9') return false;
        cents = (*cursor++ - '0') * 10;
        if (*cursor >= '0' && *cursor <= '9')
            cents += *cursor++ - '0';
    }
    if (!hasWholeDigit || *cursor != '\0') return false;

    const unsigned long long maxPaise =
        static_cast<unsigned long long>(LLONG_MAX);
    if (whole > (maxPaise - static_cast<unsigned int>(cents)) / 100)
        return false;

    const unsigned long long value = whole * 100 +
                                     static_cast<unsigned int>(cents);
    if (value == 0) return false;
    paise = static_cast<long long>(value);
    return true;
}

void formatMoney(char* output, int capacity, long long paise) {
    if (output == NULL || capacity <= 0) return;
    const long long rupees = paise / 100;
    const int cents = static_cast<int>(paise % 100);
    if (cents == 0)
        std::snprintf(output, static_cast<std::size_t>(capacity), "Rs. %lld", rupees);
    else if (cents < 10)
        std::snprintf(output, static_cast<std::size_t>(capacity), "Rs. %lld.0%d",
                      rupees, cents);
    else if (cents % 10 == 0)
        std::snprintf(output, static_cast<std::size_t>(capacity), "Rs. %lld.%d",
                      rupees, cents / 10);
    else
        std::snprintf(output, static_cast<std::size_t>(capacity), "Rs. %lld.%02d",
                      rupees, cents);
}

int buildCategoryTotals(const ExpenseManager& manager, CategoryTotal* totals,
                        int capacity) {
    if (totals == NULL || capacity <= 0) return 0;

    int count = 0;
    const ExpenseNode* node = manager.getHead();
    while (node != NULL) {
        const Expense& expense = node->getExpense();
        int index = 0;
        while (index < count &&
               std::strcmp(totals[index].name, expense.getCategory()) != 0)
            ++index;

        if (index == count) {
            if (count >= capacity) {
                node = node->getNext();
                continue;
            }
            std::snprintf(totals[count].name, sizeof(totals[count].name), "%s",
                          expense.getCategory());
            totals[count].paise = 0;
            ++count;
        }

        const long long amount = expense.getAmountPaise();
        if (amount > 0 && totals[index].paise > LLONG_MAX - amount)
            totals[index].paise = LLONG_MAX;
        else
            totals[index].paise += amount;
        node = node->getNext();
    }

    for (int i = 0; i < count; ++i) {
        for (int j = i + 1; j < count; ++j) {
            if (totals[j].paise > totals[i].paise) {
                const CategoryTotal temp = totals[i];
                totals[i] = totals[j];
                totals[j] = temp;
            }
        }
    }
    return count;
}

bool exportMonthlyReport(const ExpenseManager& manager,
                         const SavingsAdvisor& advisor,
                         const char* expenseFilePath,
                         int month, int year,
                         char* outputPath, int outputPathCapacity,
                         char* error, int errorCapacity) {
    if (outputPath != NULL && outputPathCapacity > 0) outputPath[0] = '\0';
    if (error != NULL && errorCapacity > 0) error[0] = '\0';
    if (expenseFilePath == NULL || *expenseFilePath == '\0' ||
        outputPath == NULL || outputPathCapacity <= 0 ||
        month < 1 || month > 12 || year < 1 || year > 9999) {
        setError(error, errorCapacity, "Invalid monthly report arguments.");
        return false;
    }

    const char* slash = std::strrchr(expenseFilePath, '\\');
    const char* forwardSlash = std::strrchr(expenseFilePath, '/');
    if (forwardSlash != NULL && (slash == NULL || forwardSlash > slash))
        slash = forwardSlash;
    const int directoryLength = slash == NULL ? 0 :
        static_cast<int>(slash - expenseFilePath + 1);
    if (directoryLength >= outputPathCapacity) {
        setError(error, errorCapacity, "The report path is too long.");
        return false;
    }

    if (directoryLength > 0)
        std::memcpy(outputPath, expenseFilePath,
                    static_cast<std::size_t>(directoryLength));
    const int suffixLength = std::snprintf(
        outputPath + directoryLength,
        static_cast<std::size_t>(outputPathCapacity - directoryLength),
        "monthly_report_%02d-%04d.txt", month, year);
    if (suffixLength < 0 ||
        suffixLength >= outputPathCapacity - directoryLength) {
        outputPath[0] = '\0';
        setError(error, errorCapacity, "The report path is too long.");
        return false;
    }

    ExpenseManager monthlyExpenses;
    const ExpenseNode* node = manager.getHead();
    while (node != NULL) {
        const Expense& expense = node->getExpense();
        if (matchesMonthYear(expense.getDate(), month, year) &&
            !monthlyExpenses.addLoadedExpense(expense)) {
            outputPath[0] = '\0';
            setError(error, errorCapacity,
                     "Could not prepare monthly leak analysis.");
            return false;
        }
        node = node->getNext();
    }

    ReportExporter exporter;
    if (!exporter.exportMonthlyReport(manager, month, year, outputPath)) {
        outputPath[0] = '\0';
        setError(error, errorCapacity, "Could not write the monthly transactions report.");
        return false;
    }

    std::ofstream report(outputPath, std::ios::out | std::ios::app);
    if (!report.is_open()) {
        setError(error, errorCapacity, "Could not append analysis to the monthly report.");
        return false;
    }

    LeakAnalyzer analyzer;
    analyzer.analyzeExpenses(monthlyExpenses);
    report << "\nSPENDING LEAK ANALYSIS\n";
    analyzer.printLeakReport(report);
    report << "\nFORECAST\n";
    char money[64];
    formatMoney(money, sizeof(money),
                advisor.calculateMonthlyTotal(manager, month, year));
    report << "Monthly total: " << money << "\n";
    formatMoney(money, sizeof(money),
                advisor.calculateMonthlyForecast(manager, month, year));
    report << "Forecast: " << money << "\n";
    report.flush();
    if (!report.good()) {
        setError(error, errorCapacity, "Could not finish writing the monthly report.");
        return false;
    }
    return true;
}

} // namespace GuiLogic
