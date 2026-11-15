#ifndef LEAK_ANALYZER_H
#define LEAK_ANALYZER_H

/*
 * ============================================================================
 * MODULE: LeakAnalyzer (Person 3 - HashTable & LeakAnalyzer)
 * Description: Spending leak detection engine utilizing custom HashTable
 *              to group expenses by Category:Merchant, identify repeated
 *              spending patterns, assess category volume, and analyze monthly
 *              spending distributions using actual calendar dates.
 * Policy Adherence:
 *   1. High category totals are NOT automatically labeled wasteful.
 *   2. No predictive forecasting or savings advice (owned by Person 5).
 *   3. Reads Expense records strictly through ExpenseManager::getHead() and
 *      ExpenseNode interface; never mutates expenses or clones dataset.
 *   4. Zero STL containers or algorithms.
 * ============================================================================
 */

#include "HashTable.h"
#include <iostream>

// Forward declarations to decouple header interfaces
class ExpenseManager;
class ExpenseNode;
class Expense;

class LeakAnalyzer {
public:
  static const int MAX_PATTERNS = 256;
  static const int FREQUENT_SPENDING_THRESHOLD =
      3; // >= 3 transactions is frequent
  static const int REPEATED_SPENDING_THRESHOLD =
      2; // >= 2 transactions is repeated
  static const long long HIGH_TOTAL_PAISE_THRESHOLD =
      5000000LL; // Rs. 50,000.00 in paise

private:
  HashTable m_table;         // Grouped spending by "Category:Merchant"
  HashTable m_categoryTable; // Spending totals grouped by "Category"
  HashTable m_monthlyTable;  // Spending totals grouped by "MM-YYYY"

  const HashEntry
      *m_patterns[MAX_PATTERNS]; // Identified spending leak patterns
  int m_patternCount;

  long long m_totalPaiseAnalyzed;
  int m_totalExpensesAnalyzed;

  char m_earliestDate[12]; // Format: DD-MM-YYYY
  char m_latestDate[12];   // Format: DD-MM-YYYY

  // Internal helpers
  static bool buildGroupKey(char *dest, int destSize, const char *category,
                            const char *merchant);
  static bool extractMonthYear(char *dest, int destSize, const char *dateStr);
  static long long dateToNumeric(const char *dateStr);
  void updateDateSpan(const char *dateStr);
  bool isLeakPattern(const HashEntry *entry) const;

public:
  LeakAnalyzer();
  virtual ~LeakAnalyzer();

  // Contract Public Interface
  bool analyzeExpenses(const ExpenseManager &manager);
  void displayLeakReport() const;
  int getPatternCount() const;
  const HashEntry *getPatternAt(int index) const;

  // Streamable report output (enables deterministic automated test
  // verification)
  void printLeakReport(std::ostream &os) const;

  // Inspection & Verification APIs
  const HashTable &getHashTable() const;
  const HashTable &getCategoryTable() const;
  const HashTable &getMonthlyTable() const;
  long long getTotalAnalyzedPaise() const;
  int getTotalExpensesAnalyzed() const;
  const char *getEarliestDate() const;
  const char *getLatestDate() const;
  void clear();
};

#endif // LEAK_ANALYZER_H
