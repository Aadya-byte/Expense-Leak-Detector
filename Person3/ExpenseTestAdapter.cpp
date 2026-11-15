/*
 * ============================================================================
 * TEST ADAPTER IMPLEMENTATION - EXPENSE & EXPENSE MANAGER
 * File: ExpenseTestAdapter.cpp
 *
 * NOTE: This is a test-only adapter implementing the Person 1 interface
 * contract for independent verification of Person 2's persistence module.
 * It is NOT a permanent implementation and will be replaced by Person 1's
 * Expense.cpp and ExpenseManager.cpp during full team integration.
 * ============================================================================
 */

#include "Expense.h"
#include "ExpenseManager.h"
#include <cstring>
#include <cctype>

// ----------------------------------------------------------------------------
// Expense Implementation
// ----------------------------------------------------------------------------

Expense::Expense()
    : m_id(0), m_amount(0), m_type(EXPENSE_ONE_TIME), m_frequency(FREQ_NONE) {
    m_date[0] = '\0';
    m_merchant[0] = '\0';
    m_category[0] = '\0';
}

Expense::Expense(int id, const char* date, long long amountPaise,
                 const char* merchant, const char* category,
                 ExpenseType type, ExpenseFrequency frequency)
    : m_id(id), m_amount(amountPaise), m_type(type), m_frequency(frequency) {
    setDate(date);
    setMerchant(merchant);
    setCategory(category);
}

Expense::~Expense() {
}

Expense::Expense(const Expense& other)
    : m_id(other.m_id), m_amount(other.m_amount),
      m_type(other.m_type), m_frequency(other.m_frequency) {
    std::strncpy(m_date, other.m_date, sizeof(m_date) - 1);
    m_date[sizeof(m_date) - 1] = '\0';

    std::strncpy(m_merchant, other.m_merchant, sizeof(m_merchant) - 1);
    m_merchant[sizeof(m_merchant) - 1] = '\0';

    std::strncpy(m_category, other.m_category, sizeof(m_category) - 1);
    m_category[sizeof(m_category) - 1] = '\0';
}

Expense& Expense::operator=(const Expense& other) {
    if (this != &other) {
        m_id = other.m_id;
        m_amount = other.m_amount;
        m_type = other.m_type;
        m_frequency = other.m_frequency;

        std::strncpy(m_date, other.m_date, sizeof(m_date) - 1);
        m_date[sizeof(m_date) - 1] = '\0';

        std::strncpy(m_merchant, other.m_merchant, sizeof(m_merchant) - 1);
        m_merchant[sizeof(m_merchant) - 1] = '\0';

        std::strncpy(m_category, other.m_category, sizeof(m_category) - 1);
        m_category[sizeof(m_category) - 1] = '\0';
    }
    return *this;
}

int Expense::getId() const { return m_id; }
const char* Expense::getDate() const { return m_date; }
long long Expense::getAmount() const { return m_amount; }
const char* Expense::getMerchant() const { return m_merchant; }
const char* Expense::getCategory() const { return m_category; }
ExpenseType Expense::getType() const { return m_type; }
ExpenseFrequency Expense::getFrequency() const { return m_frequency; }

const char* Expense::getTypeString() const {
    return typeToString(m_type);
}

const char* Expense::getFrequencyString() const {
    return frequencyToString(m_frequency);
}

void Expense::setId(int id) { m_id = id; }

void Expense::setDate(const char* date) {
    if (date != NULL) {
        std::strncpy(m_date, date, sizeof(m_date) - 1);
        m_date[sizeof(m_date) - 1] = '\0';
    } else {
        m_date[0] = '\0';
    }
}

void Expense::setAmount(long long amountPaise) { m_amount = amountPaise; }

void Expense::setMerchant(const char* merchant) {
    if (merchant != NULL) {
        std::strncpy(m_merchant, merchant, sizeof(m_merchant) - 1);
        m_merchant[sizeof(m_merchant) - 1] = '\0';
    } else {
        m_merchant[0] = '\0';
    }
}

void Expense::setCategory(const char* category) {
    if (category != NULL ) {
        std::strncpy(m_category, category, sizeof(m_category) - 1);
        m_category[sizeof(m_category) - 1] = '\0';
    } else {
        m_category[0] = '\0';
    }
}

void Expense::setType(ExpenseType type) { m_type = type; }

bool Expense::setType(const char* typeStr) {
    if (typeStr == NULL ) return false;
    ExpenseType t = stringToType(typeStr);
    m_type = t;
    return true;
}

void Expense::setFrequency(ExpenseFrequency frequency) { m_frequency = frequency; }

bool Expense::setFrequency(const char* freqStr) {
    if (freqStr == NULL ) return false;
    ExpenseFrequency f = stringToFrequency(freqStr);
    m_frequency = f;
    return true;
}

const char* Expense::typeToString(ExpenseType type) {
    switch (type) {
        case EXPENSE_ONE_TIME:  return "ONE_TIME";
        case EXPENSE_RECURRING: return "RECURRING";
        default:                return "ONE_TIME";
    }
}

static bool stringCaseEquals(const char* a, const char* b) {
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

ExpenseType Expense::stringToType(const char* str) {
    if (str == NULL) return EXPENSE_ONE_TIME;
    if (stringCaseEquals(str, "RECURRING")) return EXPENSE_RECURRING;
    return EXPENSE_ONE_TIME;
}

const char* Expense::frequencyToString(ExpenseFrequency freq) {
    switch (freq) {
        case FREQ_NONE:    return "NONE";
        case FREQ_DAILY:   return "DAILY";
        case FREQ_WEEKLY:  return "WEEKLY";
        case FREQ_MONTHLY: return "MONTHLY";
        case FREQ_YEARLY:  return "YEARLY";
        default:           return "NONE";
    }
}

ExpenseFrequency Expense::stringToFrequency(const char* str) {
    if (str == NULL) return FREQ_NONE;
    if (stringCaseEquals(str, "DAILY"))   return FREQ_DAILY;
    if (stringCaseEquals(str, "WEEKLY"))  return FREQ_WEEKLY;
    if (stringCaseEquals(str, "MONTHLY")) return FREQ_MONTHLY;
    if (stringCaseEquals(str, "YEARLY"))  return FREQ_YEARLY;
    return FREQ_NONE;
}

// ----------------------------------------------------------------------------
// ExpenseNode Implementation
// ----------------------------------------------------------------------------

ExpenseNode::ExpenseNode(Expense* exp, ExpenseNode* nextNode)
    : m_expense(exp), m_next(nextNode) {
}

ExpenseNode::~ExpenseNode() {
    if (m_expense != NULL) {
        delete m_expense;
        m_expense = NULL;
    }
}

Expense* ExpenseNode::getExpense() const { return m_expense; }
ExpenseNode* ExpenseNode::getNext() const { return m_next; }
void ExpenseNode::setExpense(Expense* exp) { m_expense = exp; }
void ExpenseNode::setNext(ExpenseNode* nextNode) { m_next = nextNode; }

// ----------------------------------------------------------------------------
// ExpenseManager Implementation
// ----------------------------------------------------------------------------

ExpenseManager::ExpenseManager() : m_head(NULL), m_count(0) {
}

ExpenseManager::~ExpenseManager() {
    clear();
}

ExpenseNode* ExpenseManager::getHead() const { return m_head; }
int ExpenseManager::getCount() const { return m_count; }

bool ExpenseManager::containsId(int id) const {
    ExpenseNode* curr = m_head;
    while (curr != NULL) {
        if (curr->getExpense() != NULL && curr->getExpense()->getId() == id) {
            return true;
        }
        curr = curr->getNext();
    }
    return false;
}

Expense* ExpenseManager::findExpenseById(int id) const {
    ExpenseNode* curr = m_head;
    while (curr != NULL) {
        if (curr->getExpense() != NULL && curr->getExpense()->getId() == id) {
            return curr->getExpense();
        }
        curr = curr->getNext();
    }
    return NULL;
}

bool ExpenseManager::addLoadedExpense(const Expense& expense) {
    // Preserve saved ID - reject if ID already exists
    if (containsId(expense.getId())) {
        return false;
    }

    Expense* newExp = new Expense(expense);
    ExpenseNode* newNode = new ExpenseNode(newExp, NULL);

    if (m_head == NULL) {
        m_head = newNode;
    } else {
        ExpenseNode* curr = m_head;
        while (curr->getNext() != NULL) {
            curr = curr->getNext();
        }
        curr->setNext(newNode);
    }
    m_count++;
    return true;
}

bool ExpenseManager::addLoadedExpense(Expense* expense) {
    if (expense == NULL) return false;
    if (containsId(expense->getId())) {
        return false;
    }

    ExpenseNode* newNode = new ExpenseNode(expense, NULL);
    if (m_head == NULL) {
        m_head = newNode;
    } else {
        ExpenseNode* curr = m_head;
        while (curr->getNext() != NULL) {
            curr = curr->getNext();
        }
        curr->setNext(newNode);
    }
    m_count++;
    return true;
}

bool ExpenseManager::addExpense(const Expense& expense) {
    // Auto-assign or use ID
    Expense expCopy = expense;
    if (expCopy.getId() <= 0 || containsId(expCopy.getId())) {
        // Find maximum ID + 1
        int maxId = 100;
        ExpenseNode* curr = m_head;
        while (curr != NULL) {
            if (curr->getExpense() && curr->getExpense()->getId() > maxId) {
                maxId = curr->getExpense()->getId();
            }
            curr = curr->getNext();
        }
        expCopy.setId(maxId + 1);
    }
    return addLoadedExpense(expCopy);
}

void ExpenseManager::clear() {
    ExpenseNode* curr = m_head;
    while (curr != NULL) {
        ExpenseNode* nextNode = curr->getNext();
        delete curr; // ExpenseNode destructor deletes its Expense*
        curr = nextNode;
    }
    m_head = NULL;
    m_count = 0;
}
