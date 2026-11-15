#ifndef HASH_TABLE_H
#define HASH_TABLE_H

/*
 * ============================================================================
 * MODULE: HashTable (Person 3 - HashTable & LeakAnalyzer)
 * Description: Custom fixed-capacity hash table using an ordinary contiguous
 *              array of HashEntry objects.
 * Collision Strategy: Open Addressing with Linear Probing.
 * Hash Algorithm: DJB2 (Bernstein hash) with prime-modulo distribution.
 * Constraints: Strictly zero STL containers (no map, unordered_map, vector,
 *              string, etc.) or algorithms.
 * ============================================================================
 */

#include "HashEntry.h"

class HashTable {
public:
    static const int DEFAULT_CAPACITY = 257; // Prime capacity minimizes clustering

private:
    HashEntry* m_table;         // Ordinary array of HashEntry objects
    int m_capacity;             // Fixed capacity of the table
    int m_entryCount;           // Number of distinct keys stored in the table
    int m_collisionCount;       // Total number of collisions encountered

    // Internal hash function: DJB2 algorithm
    unsigned int hashString(const char* key) const;

    // Helper for string equality without <cstring>
    static bool stringsEqual(const char* a, const char* b);

public:
    // Constructors & Destructor (Rule of Three compliant)
    HashTable();
    explicit HashTable(int capacity);
    virtual ~HashTable();
    HashTable(const HashTable& other);
    HashTable& operator=(const HashTable& other);

    // Contract Public Interface
    bool insertOrUpdate(const char* key, long long amountPaise);
    const HashEntry* find(const char* key) const;
    int getCapacity() const;
    int getEntryCount() const;
    const HashEntry* getEntryAt(int index) const;
    void clear();

    // Inspection & Metrics
    int getCollisionCount() const;
    bool isFull() const;
    double getLoadFactor() const;
};

#endif // HASH_TABLE_H
