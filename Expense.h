#ifndef EXPENSE_H
#define EXPENSE_H

class Expense {
private:
    int id;
    char date[11];
    long long amountPaise;
    char merchant[51];
    char category[31];
    char expenseType[16];
    char frequency[16];

    static bool copyText(char* destination, int capacity, const char* source);
    static bool isValidDateText(const char* value);
    static bool isAllowedExpenseType(const char* value);
    static bool isAllowedFrequency(const char* value);

public:
    Expense();
    Expense(int id, const char* date, long long amountPaise,
            const char* merchant, const char* category,
            const char* expenseType, const char* frequency);

    int getId() const;
    const char* getDate() const;
    long long getAmountPaise() const;
    const char* getMerchant() const;
    const char* getCategory() const;
    const char* getExpenseType() const;
    const char* getFrequency() const;

    bool setDate(const char* date);
    bool setAmountPaise(long long amountPaise);
    bool setMerchant(const char* merchant);
    bool setCategory(const char* category);
    bool setExpenseType(const char* expenseType);
    bool setFrequency(const char* frequency);

    bool isValid() const;
    void display() const;
};

#endif
