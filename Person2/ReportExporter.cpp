/*
 * ============================================================================
 * REPORT EXPORTER MODULE IMPLEMENTATION
 * File: ReportExporter.cpp
 * Owned by: Person 2 (Persistence & Export Module)
 * ============================================================================
 */

#include "ReportExporter.h"
#include "../Person1/ExpenseManager.h"
#include "../Person1/ExpenseNode.h"
#include "../Person1/Expense.h"

#include <fstream>
#include <iomanip>
#include <cstdio>
#include <cstring>
#include <cctype>
#include <climits>

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#include <direct.h>
#define MKDIR(dir) _mkdir(dir)
#else
#include <sys/stat.h>
#define MKDIR(dir) mkdir(dir, 0755)
#endif

ReportExporter::ReportExporter() {
}

ReportExporter::~ReportExporter() {
}

static void formatMoney(char* buffer, std::size_t capacity, long long amountPaise) {
    const long long rupees = amountPaise / 100;
    const int paise = static_cast<int>(amountPaise % 100);
    if (paise == 0)
        std::snprintf(buffer, capacity, "Rs. %lld", rupees);
    else if (paise < 10)
        std::snprintf(buffer, capacity, "Rs. %lld.0%d", rupees, paise);
    else if (paise % 10 == 0)
        std::snprintf(buffer, capacity, "Rs. %lld.%d", rupees, paise / 10);
    else
        std::snprintf(buffer, capacity, "Rs. %lld.%02d", rupees, paise);
}

const char* ReportExporter::getMonthName(int month) {
    switch (month) {
        case 1:  return "January";
        case 2:  return "February";
        case 3:  return "March";
        case 4:  return "April";
        case 5:  return "May";
        case 6:  return "June";
        case 7:  return "July";
        case 8:  return "August";
        case 9:  return "September";
        case 10: return "October";
        case 11: return "November";
        case 12: return "December";
        default: return "Unknown";
    }
}

bool ReportExporter::parseDateMonthYear(const char* dateStr, int& outMonth, int& outYear) {
    if (dateStr == NULL || std::strlen(dateStr) != 10) return false;
    if (dateStr[2] != '-' || dateStr[5] != '-') return false;

    if (!std::isdigit((unsigned char)dateStr[3]) || !std::isdigit((unsigned char)dateStr[4])) return false;
    if (!std::isdigit((unsigned char)dateStr[6]) || !std::isdigit((unsigned char)dateStr[7]) ||
        !std::isdigit((unsigned char)dateStr[8]) || !std::isdigit((unsigned char)dateStr[9])) return false;

    outMonth = (dateStr[3] - '0') * 10 + (dateStr[4] - '0');
    outYear = (dateStr[6] - '0') * 1000 + (dateStr[7] - '0') * 100 +
              (dateStr[8] - '0') * 10 + (dateStr[9] - '0');
    return true;
}

bool ReportExporter::ensureParentDirectoryExists(const char* filePath) {
    if (filePath == NULL) return false;

    const char* lastSlash = NULL;
    for (const char* p = filePath; *p != '\0'; ++p) {
        if (*p == '/' || *p == '\\') {
            lastSlash = p;
        }
    }

    if (lastSlash == NULL) return true;

    char dirPath[260];
    int len = (int)(lastSlash - filePath);
    if (len >= (int)sizeof(dirPath)) return false;

    std::strncpy(dirPath, filePath, len);
    dirPath[len] = '\0';

    for (int i = 0; i < len; ++i) {
        if (dirPath[i] == '/' || dirPath[i] == '\\') {
            char temp = dirPath[i];
            dirPath[i] = '\0';
            if (dirPath[0] != '\0') {
                MKDIR(dirPath);
            }
            dirPath[i] = temp;
        }
    }
    MKDIR(dirPath);
    return true;
}

bool ReportExporter::exportMonthlyReport(const ExpenseManager& manager,
                                         int month, int year,
                                         const char* outputPath) const {
    // Validate inputs
    if (month < 1 || month > 12) return false;
    if (year < 1 || year > 9999) return false;
    if (outputPath == NULL || *outputPath == '\0') return false;

    // Ensure output directory exists
    if (!ensureParentDirectoryExists(outputPath)) {
        return false;
    }

    // Open output file
    std::ofstream outFile(outputPath, std::ios::out | std::ios::trunc);
    if (!outFile.is_open()) {
        return false;
    }

    // Pass 1: Accumulate transactions and calculate total in paise (Zero float rounding error)
    int matchCount = 0;
    long long totalPaise = 0;

    const ExpenseNode* curr = manager.getHead();
    while (curr != NULL) {
        const Expense& exp = curr->getExpense();
        int expMonth = 0;
        int expYear = 0;
        if (parseDateMonthYear(exp.getDate(), expMonth, expYear)) {
            if (expMonth == month && expYear == year) {
                if (exp.getAmountPaise() > 0 &&
                    totalPaise > LLONG_MAX - exp.getAmountPaise()) {
                    outFile.close();
                    std::remove(outputPath);
                    return false;
                }
                matchCount++;
                totalPaise += exp.getAmountPaise();
            }
        }
        curr = curr->getNext();
    }

    outFile << "MONTHLY EXPENSE REPORT\n"
            << getMonthName(month) << " " << year << "\n"
            << "Transactions: " << matchCount << "\n\n"
            << std::left
            << std::setw(8) << "ID" << " | "
            << std::setw(10) << "Date" << " | "
            << std::setw(12) << "Amount" << " | "
            << std::setw(18) << "Category" << " | "
            << std::setw(12) << "Type" << " | "
            << std::setw(10) << "Frequency" << " | "
            << "Merchant\n"
            << "--------+------------+--------------+--------------------+--------------+------------+----------------\n";

    if (matchCount == 0) {
        outFile << "No transactions recorded for this period.\n";
    } else {
        // Pass 2: Write transaction rows
        curr = manager.getHead();
        while (curr != NULL) {
            const Expense& exp = curr->getExpense();
            int expMonth = 0;
            int expYear = 0;
            if (parseDateMonthYear(exp.getDate(), expMonth, expYear)) {
                if (expMonth == month && expYear == year) {
                    char amountText[32];
                    formatMoney(amountText, sizeof(amountText), exp.getAmountPaise());
                    outFile << std::left
                            << std::setw(8) << exp.getId() << " | "
                            << std::setw(10) << exp.getDate() << " | "
                            << std::setw(12) << amountText << " | "
                            << std::setw(18) << exp.getCategory() << " | "
                            << std::setw(12) << exp.getExpenseType() << " | "
                            << std::setw(10) << exp.getFrequency() << " | "
                            << exp.getMerchant() << "\n";
                }
            }
            curr = curr->getNext();
        }
    }

    char totalText[32];
    formatMoney(totalText, sizeof(totalText), totalPaise);
    outFile << "--------------------------------------------------------------------------------\n"
            << "Total spending: " << totalText << "\n";

    outFile.flush();
    bool success = outFile.good();
    outFile.close();
    return success;
}
