#include "Expense.h"
#include <iostream>
#include <cstring>
#include <cctype>

namespace {
bool hasNonWhitespace(const char* value) {
    if (value == 0) return false;
    while (*value != '\0') {
        if (!std::isspace(static_cast<unsigned char>(*value))) return true;
        ++value;
    }
    return false;
}
}

bool Expense::copyText(char* destination, int capacity, const char* source) {
    if (destination == 0 || source == 0 || capacity <= 0) return false;
    const std::size_t length = std::strlen(source);
    // Leave room for the terminating null byte; never silently truncate.
    if (length == 0 || length >= static_cast<std::size_t>(capacity)) return false;
    std::memcpy(destination, source, length + 1);
    return true;
}

bool Expense::isValidDateText(const char* value) {
    // Shared project contract: DD-MM-YYYY.
    if (value == 0 || std::strlen(value) != 10) return false;
    if (value[2] != '-' || value[5] != '-') return false;

    for (int i = 0; i < 10; ++i) {
        if (i == 2 || i == 5) continue;
        if (value[i] < '0' || value[i] > '9') return false;
    }

    const int day = (value[0]-'0')*10 + (value[1]-'0');
    const int month = (value[3]-'0')*10 + (value[4]-'0');
    const int year = (value[6]-'0')*1000 + (value[7]-'0')*100 +
                     (value[8]-'0')*10 + (value[9]-'0');
    if (year < 1 || month < 1 || month > 12) return false;

    const int daysInMonth[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
    int maxDay = daysInMonth[month - 1];
    const bool leap = (year % 400 == 0) || (year % 4 == 0 && year % 100 != 0);
    if (month == 2 && leap) maxDay = 29;
    return day >= 1 && day <= maxDay;
}

bool Expense::isAllowedExpenseType(const char* value) {
    return value != 0 &&
           (std::strcmp(value, "ONE_TIME") == 0 ||
            std::strcmp(value, "RECURRING") == 0);
}

bool Expense::isAllowedFrequency(const char* value) {
    return value != 0 &&
           (std::strcmp(value, "NONE") == 0 ||
            std::strcmp(value, "DAILY") == 0 ||
            std::strcmp(value, "WEEKLY") == 0 ||
            std::strcmp(value, "MONTHLY") == 0);
}

Expense::Expense() : id(0), amountPaise(-1) {
    date[0] = '\0';
    merchant[0] = '\0';
    category[0] = '\0';
    expenseType[0] = '\0';
    frequency[0] = '\0';
}

Expense::Expense(int idValue, const char* dateValue, long long amountValue,
                 const char* merchantValue, const char* categoryValue,
                 const char* expenseTypeValue, const char* frequencyValue)
    : id(idValue), amountPaise(-1) {
    date[0] = '\0';
    merchant[0] = '\0';
    category[0] = '\0';
    expenseType[0] = '\0';
    frequency[0] = '\0';

    setDate(dateValue);
    setAmountPaise(amountValue);
    setMerchant(merchantValue);
    setCategory(categoryValue);
    setExpenseType(expenseTypeValue);
    setFrequency(frequencyValue);
}

int Expense::getId() const { return id; }
const char* Expense::getDate() const { return date; }
long long Expense::getAmountPaise() const { return amountPaise; }
const char* Expense::getMerchant() const { return merchant; }
const char* Expense::getCategory() const { return category; }
const char* Expense::getExpenseType() const { return expenseType; }
const char* Expense::getFrequency() const { return frequency; }

bool Expense::setDate(const char* value) {
    if (!isValidDateText(value)) return false;
    return copyText(date, static_cast<int>(sizeof(date)), value);
}

bool Expense::setAmountPaise(long long value) {
    if (value < 0) return false;
    amountPaise = value;
    return true;
}

bool Expense::setMerchant(const char* value) {
    if (!hasNonWhitespace(value)) return false;
    return copyText(merchant, static_cast<int>(sizeof(merchant)), value);
}

bool Expense::setCategory(const char* value) {
    if (!hasNonWhitespace(value)) return false;
    return copyText(category, static_cast<int>(sizeof(category)), value);
}

bool Expense::setExpenseType(const char* value) {
    if (!hasNonWhitespace(value) || !isAllowedExpenseType(value)) return false;
    return copyText(expenseType, static_cast<int>(sizeof(expenseType)), value);
}

bool Expense::setFrequency(const char* value) {
    if (!hasNonWhitespace(value) || !isAllowedFrequency(value)) return false;
    return copyText(frequency, static_cast<int>(sizeof(frequency)), value);
}

bool Expense::isValid() const {
    return id > 0 &&
           isValidDateText(date) &&
           amountPaise >= 0 &&
           hasNonWhitespace(merchant) &&
           std::strlen(merchant) < sizeof(merchant) &&
           hasNonWhitespace(category) &&
           std::strlen(category) < sizeof(category) &&
           isAllowedExpenseType(expenseType) &&
           std::strlen(expenseType) < sizeof(expenseType) &&
           isAllowedFrequency(frequency) &&
           std::strlen(frequency) < sizeof(frequency) &&
           ((std::strcmp(expenseType, "ONE_TIME") == 0 &&
             std::strcmp(frequency, "NONE") == 0) ||
            (std::strcmp(expenseType, "RECURRING") == 0 &&
             (std::strcmp(frequency, "DAILY") == 0 ||
              std::strcmp(frequency, "WEEKLY") == 0 ||
              std::strcmp(frequency, "MONTHLY") == 0)));
}

void Expense::display() const {
    std::cout << "ID: " << id
              << " | Date: " << date
              << " | Amount: Rs. " << (amountPaise / 100) << '.';
    const long long paisePart = amountPaise >= 0 ? amountPaise % 100 : 0;
    if (paisePart < 10) std::cout << '0';
    std::cout << paisePart
              << " | Merchant: " << merchant
              << " | Category: " << category
              << " | Type: " << expenseType
              << " | Frequency: " << frequency << '\n';
}
