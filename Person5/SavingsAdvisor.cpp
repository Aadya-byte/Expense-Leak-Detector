#include "SavingsAdvisor.h"
#include "Expense.h"
#include "ExpenseNode.h"
#include <ctime>
#include <climits>

SavingsAdvisor::SavingsAdvisor() {}

static bool isValidMonthYear(int month, int year) {
    return (month >= 1 && month <= 12 && year >= 1900 && year <= 2100);
}

static bool matchesMonthYear(const char* date, int month, int year) {
    if (!date || date[2] != '-' || date[5] != '-') return false;
    for (int i = 0; i < 10; ++i) {
        if (i == 2 || i == 5) continue;
        if (date[i] < '0' || date[i] > '9') return false;
    }
    if (date[10] != '\0') return false;
    const int expenseMonth = (date[3] - '0') * 10 + date[4] - '0';
    const int expenseYear = (date[6] - '0') * 1000 + (date[7] - '0') * 100 +
                            (date[8] - '0') * 10 + date[9] - '0';
    return expenseMonth == month && expenseYear == year;
}

long long SavingsAdvisor::calculateMonthlyTotal(const ExpenseManager& manager, int month, int year) const {
    if (!isValidMonthYear(month, year)) return 0;
    
    long long total = 0;
    const ExpenseNode* expenses = manager.getHead();
    while (expenses != 0) {
        const Expense& exp = expenses->getExpense();
        if (matchesMonthYear(exp.getDate(), month, year)) {
            const long long amount = exp.getAmountPaise();
            if (amount > 0 && total > LLONG_MAX - amount) return LLONG_MAX;
            if (amount > 0) total += amount;
        }
        expenses = expenses->getNext();
    }
    return total;
}

long long SavingsAdvisor::calculateMonthlyForecast(const ExpenseManager& manager, int month, int year) const {
    if (!isValidMonthYear(month, year)) return 0;

    long long currentTotal = calculateMonthlyTotal(manager, month, year);
    if (currentTotal <= 0) return 0;

    time_t now = time(0);
    tm* ltm = localtime(&now);
    if (!ltm) return currentTotal;
    int currentYear = 1900 + ltm->tm_year;
    int currentMonth = 1 + ltm->tm_mon;
    int currentDay = ltm->tm_mday;

    if (year != currentYear || month != currentMonth) return currentTotal;
    if (currentDay <= 0) return currentTotal;

    int daysInMonth = 30;
    if (month == 1 || month == 3 || month == 5 || month == 7 || month == 8 || month == 10 || month == 12) {
        daysInMonth = 31;
    } else if (month == 2) {
        daysInMonth = ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) ? 29 : 28;
    }

    if (currentDay >= daysInMonth) return currentTotal;

    const long long wholeDays = currentTotal / currentDay;
    const long long remainingPaise = currentTotal % currentDay;
    if (wholeDays > LLONG_MAX / daysInMonth) return LLONG_MAX;
    return wholeDays * daysInMonth +
           remainingPaise * daysInMonth / currentDay;
}

long long SavingsAdvisor::calculatePotentialSavings(const ExpenseManager& manager, int month, int year, int reductionPercent) const {
    if (!isValidMonthYear(month, year) || reductionPercent < 0 || reductionPercent > 100) return 0;

    long long monthlyTotal = calculateMonthlyTotal(manager, month, year);
    return (monthlyTotal / 100) * reductionPercent +
           ((monthlyTotal % 100) * reductionPercent) / 100;
}

bool SavingsAdvisor::compareBudget(const ExpenseManager& manager, int month, int year, long long budgetPaise, long long& differencePaise) const {
    if (!isValidMonthYear(month, year) || budgetPaise < 0) return false;

    long long total = calculateMonthlyTotal(manager, month, year);
    differencePaise = budgetPaise - total;
    return (total <= budgetPaise);
}
