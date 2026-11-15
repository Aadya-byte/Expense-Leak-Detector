#ifndef HASH_ENTRY_H
#define HASH_ENTRY_H

/*
 * ============================================================================
 * MODULE: HashEntry (Person 3 - HashTable & LeakAnalyzer)
 * Description: Bounded storage cell for hash table containing grouping key,
 *              accumulated financial total in integer paise, transaction count,
 *              and spending statistics (min/max/average paise) for repeated
 *              spending detection.
 * Constraints: Strictly zero STL containers or algorithms.
 * ============================================================================
 */

#include <cstddef>

class HashEntry {
public:
  static const int MAX_KEY_LENGTH =
      128; // Bounded key length (including null terminator)

private:
  char m_key[MAX_KEY_LENGTH];
  long long m_totalPaise; // Total accumulated amount in integer paise
  int m_transactionCount; // Number of transactions recorded for this key
  long long m_minPaise;   // Minimum individual transaction amount in paise
  long long m_maxPaise;   // Maximum individual transaction amount in paise
  bool m_occupied;        // True if this slot contains active data

public:
  // Constructors
  HashEntry();
  HashEntry(const char *key, long long amountPaise);

  // Contract Interface
  bool setKey(const char *key);
  const char *getKey() const;
  long long getTotalPaise() const;
  int getTransactionCount() const;
  void addTransaction(long long amountPaise);

  // Inspection & State Management
  bool isOccupied() const;
  bool isEmpty() const;
  void clear();

  // Statistics for repeated spending pattern detection
  long long getMinPaise() const;
  long long getMaxPaise() const;
  long long getAveragePaise() const;
};

#endif // HASH_ENTRY_H
