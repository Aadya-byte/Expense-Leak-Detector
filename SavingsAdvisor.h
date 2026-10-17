#ifndef SAVINGS_ADVISOR_H
#define SAVINGS_ADVISOR_H

#include "ExpenseManager.h"

class SavingsAdvisor {
public:
    SavingsAdvisor();

    long long calculateMonthlyTotal(const ExpenseManager& manager, int month, int year) const;
    long long calculateMonthlyForecast(const ExpenseManager& manager, int month, int year) const;
    long long calculatePotentialSavings(const ExpenseManager& manager, int month, int year, int reductionPercent) const;
    bool compareBudget(const ExpenseManager& manager, int month, int year, long long budgetPaise, long long& differencePaise) const;
};

#endif // SAVINGS_ADVISOR_H
