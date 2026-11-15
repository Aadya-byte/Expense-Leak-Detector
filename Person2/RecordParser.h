#ifndef RECORD_PARSER_H
#define RECORD_PARSER_H

/*
 * ============================================================================
 * RECORD PARSER MODULE
 * Owned by: Person 2 (Persistence & Export Module)
 *
 * Responsibilities:
 * - Parse pipe-delimited expense records into Expense objects.
 * - Validate all seven fields: ID, Date, Amount, Merchant, Category, Type, Frequency.
 * - Enforce field length constraints, numeric boundaries, calendar rules, and
 *   type/frequency consistency.
 * - Reject any record containing misplaced delimiter '|' or malformed values.
 * - Zero STL container dependency.
 * ============================================================================
 */

#include <cstddef>

// Forward declaration of Expense to maintain strict decoupling
class Expense;

class RecordParser {
public:
    RecordParser();
    ~RecordParser();

    // Parse a single pipe-delimited line into the output Expense object.
    // Returns true on success; false if the record is malformed or invalid.
    bool parseRecord(const char* line, Expense& outputExpense) const;

    // Validate all seven fields of an already constructed Expense object.
    // Returns true if all fields conform to the contract rules; false otherwise.
    bool validateRecord(const Expense& expense) const;

private:
    // Helper validation routines
    bool isValidDate(const char* dateStr, int& outDay, int& outMonth, int& outYear) const;
    bool isLeapYear(int year) const;
    int getDaysInMonth(int month, int year) const;
    bool parseAmount(const char* amountStr, long long& outPaise) const;
    bool parsePositiveInt(const char* str, int& outVal) const;
    bool isValidType(const char* typeStr) const;
    bool isValidFrequency(const char* freqStr, const char* typeStr) const;
    static void trimWhitespace(char* str);
    static bool isAllWhitespace(const char* str);
};

#endif // RECORD_PARSER_H
