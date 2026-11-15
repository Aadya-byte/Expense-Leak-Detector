#ifndef GUI_LOGIC_H
#define GUI_LOGIC_H

#include "ExpenseManager.h"

class SavingsAdvisor;

struct CategoryTotal {
    char name[31];
    long long paise;
};

namespace GuiLogic {

bool containsIgnoreCase(const char* text, const char* query);
void trim(char* text);
bool parseRupees(const char* text, long long& paise);
void formatMoney(char* output, int capacity, long long paise);
int buildCategoryTotals(const ExpenseManager& manager, CategoryTotal* totals,
                        int capacity);
bool exportMonthlyReport(const ExpenseManager& manager,
                         const SavingsAdvisor& advisor,
                         const char* expenseFilePath,
                         int month, int year,
                         char* outputPath, int outputPathCapacity,
                         char* error, int errorCapacity);

} // namespace GuiLogic

#endif
