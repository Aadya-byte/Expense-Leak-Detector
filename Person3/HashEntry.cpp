#include "HashEntry.h"

/*
 * ============================================================================
 * MODULE: HashEntry Implementation
 * Constraints: Strictly zero STL containers or algorithms.
 * ============================================================================
 */

HashEntry::HashEntry()
    : m_totalPaise(0),
      m_transactionCount(0),
      m_minPaise(0),
      m_maxPaise(0),
      m_occupied(false) {
    m_key[0] = '\0';
}

HashEntry::HashEntry(const char* key, long long amountPaise)
    : m_totalPaise(0),
      m_transactionCount(0),
      m_minPaise(0),
      m_maxPaise(0),
      m_occupied(false) {
    m_key[0] = '\0';
    if (setKey(key)) {
        addTransaction(amountPaise);
    }
}

bool HashEntry::setKey(const char* key) {
    if (key == NULL) {
        return false;
    }

    // Measure key length and validate bound
    int len = 0;
    while (key[len] != '\0') {
        len++;
        if (len >= MAX_KEY_LENGTH) {
            return false; // Key exceeds bounded array size
        }
    }

    if (len == 0) {
        return false; // Empty key rejected
    }

    // Safely copy key including null terminator
    for (int i = 0; i <= len; ++i) {
        m_key[i] = key[i];
    }

    return true;
}

const char* HashEntry::getKey() const {
    return m_key;
}

long long HashEntry::getTotalPaise() const {
    return m_totalPaise;
}

int HashEntry::getTransactionCount() const {
    return m_transactionCount;
}

void HashEntry::addTransaction(long long amountPaise) {
    // Validate numeric inputs (only positive amounts are allowed for expenses)
    if (amountPaise <= 0) {
        return;
    }

    // Guard against 64-bit integer overflow
    const long long MAX_PAISE = 0x7FFFFFFFFFFFFFFFLL;
    if (m_totalPaise > MAX_PAISE - amountPaise) {
        return;
    }

    // Update spending statistics
    if (m_transactionCount == 0) {
        m_minPaise = amountPaise;
        m_maxPaise = amountPaise;
    } else {
        if (amountPaise < m_minPaise) {
            m_minPaise = amountPaise;
        }
        if (amountPaise > m_maxPaise) {
            m_maxPaise = amountPaise;
        }
    }

    m_totalPaise += amountPaise;
    m_transactionCount++;
    m_occupied = true;
}

bool HashEntry::isOccupied() const {
    return m_occupied;
}

bool HashEntry::isEmpty() const {
    return !m_occupied;
}

void HashEntry::clear() {
    m_key[0] = '\0';
    m_totalPaise = 0;
    m_transactionCount = 0;
    m_minPaise = 0;
    m_maxPaise = 0;
    m_occupied = false;
}

long long HashEntry::getMinPaise() const {
    return m_minPaise;
}

long long HashEntry::getMaxPaise() const {
    return m_maxPaise;
}

long long HashEntry::getAveragePaise() const {
    if (m_transactionCount <= 0) {
        return 0;
    }
    return m_totalPaise / m_transactionCount;
}
