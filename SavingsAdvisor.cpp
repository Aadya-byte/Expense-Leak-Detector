#include "SavingsAdvisor.h"
#include <ctime>

SavingsAdvisor::SavingsAdvisor() {}

static bool isValidMonthYear(int month, int year) {
    return (month >= 1 && month <= 12 && year >= 1900 && year <= 2100);
}

long long SavingsAdvisor::calculateMonthlyTotal(const ExpenseManager& manager, int month, int year) const {
    if (!isValidMonthYear(month, year)) return 0;
    
    long long total = 0;
    auto expenses = manager.getAllExpenses();
    for (const auto& exp : expenses) {
        if (exp.getMonth() == month && exp.getYear() == year) {
            total += exp.getAmountPaise();
        }
    }
    return total;
}

long long SavingsAdvisor::calculateMonthlyForecast(const ExpenseManager& manager, int month, int year) const {
    if (!isValidMonthYear(month, year)) return 0;

    long long currentTotal = calculateMonthlyTotal(manager, month, year);
    if (currentTotal <= 0) return 0;

    time_t now = time(0);
    tm* ltm = localtime(&now);
    int currentYear = 1900 + ltm->tm_year;
    int currentMonth = 1 + ltm->tm_mon;
    int currentDay = ltm->tm_mday;

    if (year != currentYear || month != currentMonth) {
        return currentTotal;
    }

    if (currentDay <= 0) return currentTotal;

    int daysInMonth = 30;
    if (month == 1 || month == 3 || month == 5 || month == 7 || month == 8 || month == 10 || month == 12) {
        daysInMonth = 31;
    } else if (month == 2) {
        daysInMonth = ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) ? 29 : 28;
    }

    if (currentDay >= daysInMonth) return currentTotal;

    double dailyRunRate = static_cast<double>(currentTotal) / currentDay;
    return static_cast<long long>(dailyRunRate * daysInMonth);
}

long long SavingsAdvisor::calculatePotentialSavings(const ExpenseManager& manager, int month, int year, int reductionPercent) const {
    if (!isValidMonthYear(month, year) || reductionPercent < 0 || reductionPercent > 100) return 0;

    long long monthlyTotal = calculateMonthlyTotal(manager, month, year);
    return (monthlyTotal * reductionPercent) / 100;
}

bool SavingsAdvisor::compareBudget(const ExpenseManager& manager, int month, int year, long long budgetPaise, long long& differencePaise) const {
    if (!isValidMonthYear(month, year) || budgetPaise < 0) return false;

    long long total = calculateMonthlyTotal(manager, month, year);
    differencePaise = budgetPaise - total;
    return (total <= budgetPaise);
}
