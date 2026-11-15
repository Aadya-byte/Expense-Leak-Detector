/*
 * ============================================================================
 * RECORD PARSER MODULE IMPLEMENTATION
 * File: RecordParser.cpp
 * Owned by: Person 2 (Persistence & Export Module)
 * ============================================================================
 */

#include "RecordParser.h"
#include "../Person1/Expense.h"
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <climits>

RecordParser::RecordParser() {
}

RecordParser::~RecordParser() {
}

void RecordParser::trimWhitespace(char* str) {
    if (str == NULL) return;
    int len = (int)std::strlen(str);
    if (len == 0) return;

    // Trim trailing whitespace and carriage returns/newlines
    int end = len - 1;
    while (end >= 0 && (str[end] == ' ' || str[end] == '\t' || str[end] == '\r' || str[end] == '\n')) {
        str[end] = '\0';
        end--;
    }

    // Trim leading whitespace
    int start = 0;
    while (str[start] != '\0' && (str[start] == ' ' || str[start] == '\t' || str[start] == '\r' || str[start] == '\n')) {
        start++;
    }

    if (start > 0) {
        int i = 0;
        while (str[start] != '\0') {
            str[i++] = str[start++];
        }
        str[i] = '\0';
    }
}

bool RecordParser::isAllWhitespace(const char* str) {
    if (str == NULL) return true;
    while (*str) {
        if (!std::isspace((unsigned char)*str)) {
            return false;
        }
        str++;
    }
    return true;
}

static bool strCaseCmp(const char* a, const char* b) {
    if (a == NULL || b == NULL) return false;
    while (*a && *b) {
        char ca = *a;
        char cb = *b;
        if (ca >= 'a' && ca <= 'z') ca = (char)(ca - 'a' + 'A');
        if (cb >= 'a' && cb <= 'z') cb = (char)(cb - 'a' + 'A');
        if (ca == '-' || ca == ' ') ca = '_';
        if (cb == '-' || cb == ' ') cb = '_';
        if (ca != cb) return false;
        a++;
        b++;
    }
    return (*a == '\0' && *b == '\0');
}

bool RecordParser::isLeapYear(int year) const {
    return ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0));
}

int RecordParser::getDaysInMonth(int month, int year) const {
    switch (month) {
        case 1:  return 31;
        case 2:  return isLeapYear(year) ? 29 : 28;
        case 3:  return 31;
        case 4:  return 30;
        case 5:  return 31;
        case 6:  return 30;
        case 7:  return 31;
        case 8:  return 31;
        case 9:  return 30;
        case 10: return 31;
        case 11: return 30;
        case 12: return 31;
        default: return 0;
    }
}

bool RecordParser::isValidDate(const char* dateStr, int& outDay, int& outMonth, int& outYear) const {
    if (dateStr == NULL) return false;
    if (std::strlen(dateStr) != 10) return false;

    // Required format: DD-MM-YYYY
    if (dateStr[2] != '-' || dateStr[5] != '-') return false;

    for (int i = 0; i < 10; ++i) {
        if (i == 2 || i == 5) continue;
        if (!std::isdigit((unsigned char)dateStr[i])) return false;
    }

    int day = (dateStr[0] - '0') * 10 + (dateStr[1] - '0');
    int month = (dateStr[3] - '0') * 10 + (dateStr[4] - '0');
    int year = (dateStr[6] - '0') * 1000 + (dateStr[7] - '0') * 100 +
               (dateStr[8] - '0') * 10 + (dateStr[9] - '0');

    if (year < 1) return false;
    if (month < 1 || month > 12) return false;
    int maxDays = getDaysInMonth(month, year);
    if (day < 1 || day > maxDays) return false;

    outDay = day;
    outMonth = month;
    outYear = year;
    return true;
}

bool RecordParser::parsePositiveInt(const char* str, int& outVal) const {
    if (str == NULL || *str == '\0') return false;
    int val = 0;
    for (int i = 0; str[i] != '\0'; ++i) {
        if (!std::isdigit((unsigned char)str[i])) return false;
        int digit = str[i] - '0';
        if (val > (INT_MAX - digit) / 10) return false;
        val = val * 10 + digit;
    }
    if (val <= 0) return false;
    outVal = val;
    return true;
}

bool RecordParser::parseAmount(const char* amountStr, long long& outPaise) const {
    if (amountStr == NULL || *amountStr == '\0') return false;

    // Check if decimal point exists
    const char* dotPos = std::strchr(amountStr, '.');
    if (dotPos == NULL) {
        // Pure integer paise representation (e.g. "15000" for Rs 150.00)
        int len = (int)std::strlen(amountStr);
        if (len == 0) return false;
        for (int i = 0; i < len; ++i) {
            if (!std::isdigit((unsigned char)amountStr[i])) return false;
        }
        long long val = 0;
        for (int i = 0; i < len; ++i) {
            int digit = amountStr[i] - '0';
            if (val > (LLONG_MAX - digit) / 10) return false;
            val = val * 10 + digit;
        }
        if (val <= 0) return false;
        outPaise = val;
        return true;
    } else {
        // Decimal rupees representation (e.g. "150.00" or "150.5")
        // Check for multiple dots
        if (std::strchr(dotPos + 1, '.') != NULL) return false;

        // Parse whole rupees portion before dot
        int rupeeLen = (int)(dotPos - amountStr);
        if (rupeeLen == 0) return false;
        for (int i = 0; i < rupeeLen; ++i) {
            if (!std::isdigit((unsigned char)amountStr[i])) return false;
        }

        long long rupees = 0;
        for (int i = 0; i < rupeeLen; ++i) {
            int digit = amountStr[i] - '0';
            if (rupees > (LLONG_MAX / 100 - digit) / 10) return false;
            rupees = rupees * 10 + digit;
        }

        // Parse fraction portion after dot (must be 1 or 2 digits)
        const char* fracStr = dotPos + 1;
        int fracLen = (int)std::strlen(fracStr);
        if (fracLen < 1 || fracLen > 2) return false;

        int paisePart = 0;
        if (fracLen == 1) {
            if (!std::isdigit((unsigned char)fracStr[0])) return false;
            paisePart = (fracStr[0] - '0') * 10;
        } else if (fracLen == 2) {
            if (!std::isdigit((unsigned char)fracStr[0]) ||
                !std::isdigit((unsigned char)fracStr[1])) return false;
            paisePart = (fracStr[0] - '0') * 10 + (fracStr[1] - '0');
        }

        if (rupees > (LLONG_MAX - paisePart) / 100) return false;
        long long totalPaise = rupees * 100 + paisePart;
        if (totalPaise <= 0) return false;
        outPaise = totalPaise;
        return true;
    }
}

bool RecordParser::isValidType(const char* typeStr) const {
    if (typeStr == NULL || *typeStr == '\0') return false;
    if (strCaseCmp(typeStr, "ONE_TIME") || strCaseCmp(typeStr, "RECURRING")) {
        return true;
    }
    return false;
}

bool RecordParser::isValidFrequency(const char* freqStr, const char* typeStr) const {
    if (freqStr == NULL || typeStr == NULL) return false;

    if (strCaseCmp(typeStr, "ONE_TIME")) {
        return strCaseCmp(freqStr, "NONE");
    } else if (strCaseCmp(typeStr, "RECURRING")) {
        return strCaseCmp(freqStr, "DAILY") ||
               strCaseCmp(freqStr, "WEEKLY") ||
               strCaseCmp(freqStr, "MONTHLY");
    }
    return false;
}

bool RecordParser::validateRecord(const Expense& expense) const {
    // 1. ID must be positive
    if (expense.getId() <= 0) return false;

    // 2. Date must be valid DD-MM-YYYY calendar date
    int d, m, y;
    if (!isValidDate(expense.getDate(), d, m, y)) return false;

    // 3. Amount must be positive integer paise
    if (expense.getAmountPaise() <= 0) return false;

    // 4. Merchant: non-empty, within length, no '|'
    const char* merchant = expense.getMerchant();
    if (merchant == NULL || *merchant == '\0') return false;
    if (std::strlen(merchant) >= 51) return false;
    if (std::strchr(merchant, '|') != NULL) return false;
    if (isAllWhitespace(merchant)) return false;

    // 5. Category: non-empty, within length, no '|'
    const char* category = expense.getCategory();
    if (category == NULL || *category == '\0') return false;
    if (std::strlen(category) >= 31) return false;
    if (std::strchr(category, '|') != NULL) return false;
    if (isAllWhitespace(category)) return false;

    // 6. Type: valid enum / string
    const char* typeStr = expense.getExpenseType();
    if (!isValidType(typeStr)) return false;

    // 7. Frequency: valid and compatible with Type
    const char* freqStr = expense.getFrequency();
    if (!isValidFrequency(freqStr, typeStr)) return false;

    return true;
}

bool RecordParser::parseRecord(const char* line, Expense& outputExpense) const {
    if (line == NULL) return false;

    // Count pipes: exactly 6 pipes needed for 7 fields
    int pipeCount = 0;
    const char* ptr = line;
    while (*ptr != '\0') {
        if (*ptr == '\r' || *ptr == '\n') break;
        if (*ptr == '|') pipeCount++;
        ptr++;
    }
    if (pipeCount != 6) {
        return false;
    }

    // Extract 7 fields into fixed-size character buffers (No STL)
    char fields[7][128];
    int currentField = 0;
    int fieldCharIdx = 0;
    ptr = line;

    while (*ptr != '\0' && *ptr != '\r' && *ptr != '\n') {
        if (*ptr == '|') {
            fields[currentField][fieldCharIdx] = '\0';
            currentField++;
            fieldCharIdx = 0;
            if (currentField > 6) return false;
        } else {
            if (fieldCharIdx >= 127) {
                // Field length exceeds maximum permissible buffer
                return false;
            }
            fields[currentField][fieldCharIdx++] = *ptr;
        }
        ptr++;
    }
    fields[currentField][fieldCharIdx] = '\0';

    if (currentField != 6) {
        return false;
    }

    // Trim whitespace from each field
    for (int i = 0; i < 7; ++i) {
        trimWhitespace(fields[i]);
    }

    // Field 1: ID (Numeric positive integer)
    int id = 0;
    if (!parsePositiveInt(fields[0], id)) {
        return false;
    }

    // Field 2: Date (DD-MM-YYYY format and calendar check)
    int day, month, year;
    if (!isValidDate(fields[1], day, month, year)) {
        return false;
    }

    // Field 3: Amount (Integer paise, positive)
    long long amountPaise = 0;
    if (!parseAmount(fields[2], amountPaise)) {
        return false;
    }

    // Field 4: Merchant (1-50 chars, no '|', not all whitespace)
    if (fields[3][0] == '\0' || isAllWhitespace(fields[3]) || std::strlen(fields[3]) >= 51) {
        return false;
    }
    if (std::strchr(fields[3], '|') != NULL) {
        return false;
    }

    // Field 5: Category (1-30 chars, no '|', not all whitespace)
    if (fields[4][0] == '\0' || isAllWhitespace(fields[4]) || std::strlen(fields[4]) >= 31) {
        return false;
    }
    if (std::strchr(fields[4], '|') != NULL) {
        return false;
    }

    // Field 6: Type (ONE_TIME or RECURRING)
    if (!isValidType(fields[5])) {
        return false;
    }

    // Field 7: Frequency (Consistent with Type)
    if (!isValidFrequency(fields[6], fields[5])) {
        return false;
    }

    Expense parsedExpense(id, fields[1], amountPaise, fields[3], fields[4],
                          fields[5], fields[6]);
    if (!validateRecord(parsedExpense)) return false;
    outputExpense = parsedExpense;
    return true;
}
