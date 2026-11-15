#include "HashTable.h"
#include <climits>

/*
 * ============================================================================
 * MODULE: HashTable Implementation
 * Collision Strategy: Linear Probing
 * Constraints: Strictly zero STL containers or algorithms.
 * ============================================================================
 */

HashTable::HashTable()
    : m_capacity(DEFAULT_CAPACITY),
      m_entryCount(0),
      m_collisionCount(0) {
    m_table = new HashEntry[m_capacity];
}

HashTable::HashTable(int capacity)
    : m_entryCount(0),
      m_collisionCount(0) {
    if (capacity <= 0) {
        m_capacity = DEFAULT_CAPACITY;
    } else {
        m_capacity = capacity;
    }
    m_table = new HashEntry[m_capacity];
}

HashTable::~HashTable() {
    if (m_table != NULL) {
        delete[] m_table;
        m_table = NULL;
    }
}

HashTable::HashTable(const HashTable& other)
    : m_capacity(other.m_capacity),
      m_entryCount(other.m_entryCount),
      m_collisionCount(other.m_collisionCount) {
    m_table = new HashEntry[m_capacity];
    for (int i = 0; i < m_capacity; ++i) {
        m_table[i] = other.m_table[i];
    }
}

HashTable& HashTable::operator=(const HashTable& other) {
    if (this != &other) {
        if (m_table != NULL) {
            delete[] m_table;
        }
        m_capacity = other.m_capacity;
        m_entryCount = other.m_entryCount;
        m_collisionCount = other.m_collisionCount;
        m_table = new HashEntry[m_capacity];
        for (int i = 0; i < m_capacity; ++i) {
            m_table[i] = other.m_table[i];
        }
    }
    return *this;
}

bool HashTable::stringsEqual(const char* a, const char* b) {
    if (a == NULL || b == NULL) {
        return (a == b);
    }
    while (*a != '\0' && *b != '\0') {
        if (*a != *b) {
            return false;
        }
        a++;
        b++;
    }
    return (*a == '\0' && *b == '\0');
}

unsigned int HashTable::hashString(const char* key) const {
    if (key == NULL || m_capacity <= 0) {
        return 0;
    }

    // DJB2 Hash Algorithm (Daniel J. Bernstein)
    unsigned long hash = 5381;
    int c = 0;
    while ((c = static_cast<unsigned char>(*key++))) {
        // hash * 33 + c
        hash = ((hash << 5) + hash) + static_cast<unsigned long>(c);
    }

    return static_cast<unsigned int>(hash % static_cast<unsigned long>(m_capacity));
}

bool HashTable::insertOrUpdate(const char* key, long long amountPaise) {
    // Input validation: key presence, length, and positive monetary amount
    if (key == NULL || key[0] == '\0') {
        return false;
    }

    int len = 0;
    while (key[len] != '\0') {
        len++;
        if (len >= HashEntry::MAX_KEY_LENGTH) {
            return false; // Key length exceeds buffer capacity
        }
    }

    if (amountPaise <= 0) {
        return false; // Non-positive amount rejected
    }

    if (m_capacity <= 0 || m_table == NULL) {
        return false;
    }

    unsigned int initialIndex = hashString(key);

    // Linear Probing: examine successive slots wrapping circularly
    for (int step = 0; step < m_capacity; ++step) {
        int slot = static_cast<int>((initialIndex + static_cast<unsigned int>(step)) % static_cast<unsigned int>(m_capacity));

        if (!m_table[slot].isOccupied()) {
            // Found empty slot: verify capacity before insertion
            if (m_entryCount >= m_capacity) {
                return false; // Table completely full
            }

            if (!m_table[slot].setKey(key)) {
                return false;
            }
            m_table[slot].addTransaction(amountPaise);
            m_entryCount++;
            return true;
        }

        // Slot occupied: check if key matches existing entry
        if (stringsEqual(m_table[slot].getKey(), key)) {
            // Duplicate/repeated key: accumulate into existing entry
            if (m_table[slot].getTotalPaise() > LLONG_MAX - amountPaise ||
                m_table[slot].getTransactionCount() == INT_MAX) {
                return false;
            }
            m_table[slot].addTransaction(amountPaise);
            return true;
        }

        // Collision occurred (slot occupied by a different key)
        m_collisionCount++;
    }

    // All slots checked and no empty slot found
    return false;
}

const HashEntry* HashTable::find(const char* key) const {
    if (key == NULL || key[0] == '\0' || m_capacity <= 0 || m_table == NULL) {
        return NULL;
    }

    unsigned int initialIndex = hashString(key);

    for (int step = 0; step < m_capacity; ++step) {
        int slot = static_cast<int>((initialIndex + static_cast<unsigned int>(step)) % static_cast<unsigned int>(m_capacity));

        if (!m_table[slot].isOccupied()) {
            // Open addressing invariant: hitting an empty slot guarantees the key does not exist
            return NULL;
        }

        if (stringsEqual(m_table[slot].getKey(), key)) {
            return &m_table[slot];
        }
    }

    return NULL;
}

int HashTable::getCapacity() const {
    return m_capacity;
}

int HashTable::getEntryCount() const {
    return m_entryCount;
}

const HashEntry* HashTable::getEntryAt(int index) const {
    if (index < 0 || index >= m_capacity || m_table == NULL) {
        return NULL;
    }
    return &m_table[index];
}

void HashTable::clear() {
    if (m_table != NULL) {
        for (int i = 0; i < m_capacity; ++i) {
            m_table[i].clear();
        }
    }
    m_entryCount = 0;
    m_collisionCount = 0;
}

int HashTable::getCollisionCount() const {
    return m_collisionCount;
}

bool HashTable::isFull() const {
    return m_entryCount >= m_capacity;
}

double HashTable::getLoadFactor() const {
    if (m_capacity <= 0) {
        return 0.0;
    }
    return static_cast<double>(m_entryCount) / static_cast<double>(m_capacity);
}
