#ifndef EXPENSE_H
#define EXPENSE_H

/*
 * ============================================================================
 * TEAM CONTRACT SPECIFICATION - EXPENSE & EXPENSE NODE INTERFACE
 * Owned by: Person 1 (Core Model & Singly Linked List)
 * Referenced by: Person 2 (Persistence & Export Module)
 *
 * NOTE: This header defines the exact contract interface for Expense and
 * ExpenseNode agreed upon in Section 4 and Section 8 of the team contract.
 * ============================================================================
 */

#include <cstddef>

enum ExpenseType {
    EXPENSE_ONE_TIME = 0,
    EXPENSE_RECURRING = 1
};

enum ExpenseFrequency {
    FREQ_NONE = 0,
    FREQ_DAILY = 1,
    FREQ_WEEKLY = 2,
    FREQ_MONTHLY = 3,
    FREQ_YEARLY = 4
};

class Expense {
private:
    int m_id;
    char m_date[12];        // DD-MM-YYYY format (null-terminated)
    long long m_amount;     // Stored in integer paise (15000 = Rs. 150.00)
    char m_merchant[64];    // Max 63 chars + null terminator
    char m_category[64];    // Max 63 chars + null terminator
    ExpenseType m_type;
    ExpenseFrequency m_frequency;

public:
    Expense();
    Expense(int id, const char* date, long long amountPaise,
            const char* merchant, const char* category,
            ExpenseType type = EXPENSE_ONE_TIME,
            ExpenseFrequency frequency = FREQ_NONE);
    virtual ~Expense();

    // Copy constructor & assignment
    Expense(const Expense& other);
    Expense& operator=(const Expense& other);

    // Getters
    int getId() const;
    const char* getDate() const;
    long long getAmount() const; // Value in integer paise
    const char* getMerchant() const;
    const char* getCategory() const;
    ExpenseType getType() const;
    const char* getTypeString() const;
    ExpenseFrequency getFrequency() const;
    const char* getFrequencyString() const;

    // Setters
    void setId(int id);
    void setDate(const char* date);
    void setAmount(long long amountPaise);
    void setMerchant(const char* merchant);
    void setCategory(const char* category);
    void setType(ExpenseType type);
    bool setType(const char* typeStr);
    void setFrequency(ExpenseFrequency frequency);
    bool setFrequency(const char* freqStr);

    // Conversion helpers
    static const char* typeToString(ExpenseType type);
    static ExpenseType stringToType(const char* str);
    static const char* frequencyToString(ExpenseFrequency freq);
    static ExpenseFrequency stringToFrequency(const char* str);
};

// Node for singly linked list in ExpenseManager
class ExpenseNode {
private:
    Expense* m_expense;
    ExpenseNode* m_next;

public:
    ExpenseNode(Expense* exp = NULL , ExpenseNode* nextNode = NULL);
    ~ExpenseNode();

    Expense* getExpense() const;
    ExpenseNode* getNext() const;
    void setExpense(Expense* exp);
    void setNext(ExpenseNode* nextNode);
};

#endif // EXPENSE_H
