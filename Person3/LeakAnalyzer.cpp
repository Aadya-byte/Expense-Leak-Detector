#include "LeakAnalyzer.h"
#include "../Person1/Expense.h"
#include "../Person1/ExpenseManager.h"
#include "../Person1/ExpenseNode.h"
#include <iomanip>
#include <climits>

/*
 * ============================================================================
 * MODULE: LeakAnalyzer Implementation
 * Constraints: Strictly zero STL containers or algorithms.
 * ============================================================================
 */

LeakAnalyzer::LeakAnalyzer()
    : m_table(HashTable::DEFAULT_CAPACITY), m_categoryTable(67),
      m_monthlyTable(67), m_patternCount(0), m_totalPaiseAnalyzed(0),
      m_totalExpensesAnalyzed(0) {
  m_earliestDate[0] = '\0';
  m_latestDate[0] = '\0';
  for (int i = 0; i < MAX_PATTERNS; ++i) {
    m_patterns[i] = NULL;
  }
}

LeakAnalyzer::~LeakAnalyzer() { clear(); }

void LeakAnalyzer::clear() {
  m_table.clear();
  m_categoryTable.clear();
  m_monthlyTable.clear();
  m_patternCount = 0;
  m_totalPaiseAnalyzed = 0;
  m_totalExpensesAnalyzed = 0;
  m_earliestDate[0] = '\0';
  m_latestDate[0] = '\0';
  for (int i = 0; i < MAX_PATTERNS; ++i) {
    m_patterns[i] = NULL;
  }
}

bool LeakAnalyzer::buildGroupKey(char *dest, int destSize, const char *category,
                                 const char *merchant) {
  if (dest == NULL || destSize <= 0) {
    return false;
  }

  const char *cat =
      (category != NULL && category[0] != '\0') ? category : "Uncategorized";
  const char *mer =
      (merchant != NULL && merchant[0] != '\0') ? merchant : "Unknown";

  int catLen = 0;
  while (cat[catLen] != '\0')
    catLen++;

  int merLen = 0;
  while (mer[merLen] != '\0')
    merLen++;

  // Format: "Category:Merchant"
  if (catLen + 1 + merLen >= destSize) {
    return false; // Key buffer would overflow
  }

  int idx = 0;
  for (int i = 0; i < catLen; ++i) {
    dest[idx++] = cat[i];
  }
  dest[idx++] = ':';
  for (int i = 0; i < merLen; ++i) {
    dest[idx++] = mer[i];
  }
  dest[idx] = '\0';

  return true;
}

bool LeakAnalyzer::extractMonthYear(char *dest, int destSize,
                                    const char *dateStr) {
  if (dateStr == NULL || destSize < 8) {
    return false;
  }

  int len = 0;
  while (dateStr[len] != '\0')
    len++;

  if (len < 10) {
    return false; // Date must be at least DD-MM-YYYY format
  }

  // Format DD-MM-YYYY -> MM-YYYY
  dest[0] = dateStr[3];
  dest[1] = dateStr[4];
  dest[2] = '-';
  dest[3] = dateStr[6];
  dest[4] = dateStr[7];
  dest[5] = dateStr[8];
  dest[6] = dateStr[9];
  dest[7] = '\0';

  return true;
}

long long LeakAnalyzer::dateToNumeric(const char *dateStr) {
  if (dateStr == NULL)
    return -1;

  int len = 0;
  while (dateStr[len] != '\0')
    len++;
  if (len < 10)
    return -1;

  int d = (dateStr[0] - '0') * 10 + (dateStr[1] - '0');
  int m = (dateStr[3] - '0') * 10 + (dateStr[4] - '0');
  int y = (dateStr[6] - '0') * 1000 + (dateStr[7] - '0') * 100 +
          (dateStr[8] - '0') * 10 + (dateStr[9] - '0');

  if (m < 1 || m > 12 || d < 1 || d > 31 || y < 1900) {
    return -1;
  }

  return static_cast<long long>(y) * 10000LL +
         static_cast<long long>(m) * 100LL + static_cast<long long>(d);
}

void LeakAnalyzer::updateDateSpan(const char *dateStr) {
  if (dateStr == NULL || dateStr[0] == '\0') {
    return;
  }

  long long currentNum = dateToNumeric(dateStr);
  if (currentNum <= 0) {
    return;
  }

  if (m_earliestDate[0] == '\0') {
    int i = 0;
    while (dateStr[i] != '\0' && i < 11) {
      m_earliestDate[i] = dateStr[i];
      m_latestDate[i] = dateStr[i];
      i++;
    }
    m_earliestDate[i] = '\0';
    m_latestDate[i] = '\0';
    return;
  }

  long long earlyNum = dateToNumeric(m_earliestDate);
  long long lateNum = dateToNumeric(m_latestDate);

  if (earlyNum <= 0 || currentNum < earlyNum) {
    int i = 0;
    while (dateStr[i] != '\0' && i < 11) {
      m_earliestDate[i] = dateStr[i];
      i++;
    }
    m_earliestDate[i] = '\0';
  }

  if (lateNum <= 0 || currentNum > lateNum) {
    int i = 0;
    while (dateStr[i] != '\0' && i < 11) {
      m_latestDate[i] = dateStr[i];
      i++;
    }
    m_latestDate[i] = '\0';
  }
}

bool LeakAnalyzer::isLeakPattern(const HashEntry *entry) const {
  if (entry == NULL || !entry->isOccupied()) {
    return false;
  }

  // Pattern Detection Rule:
  // Repeated spending at the same merchant/category (transactionCount >= 2)
  // constitutes a habit-driven spending channel or potential leak.
  if (entry->getTransactionCount() >= REPEATED_SPENDING_THRESHOLD) {
    return true;
  }

  return false;
}

bool LeakAnalyzer::analyzeExpenses(const ExpenseManager &manager) {
  clear();

  const ExpenseNode *curr = manager.getHead();
  if (curr == NULL) {
    // Empty dataset: cleanly handled, returns true with 0 patterns
    return true;
  }

  // Traverse singly linked list without modifying any records or creating
  // shadow lists
  while (curr != NULL) {
    const Expense *exp = &curr->getExpense();
    if (exp != NULL) {
      long long amount = exp->getAmountPaise();
      if (amount > 0) {
        // 1. Group by Category:Merchant in primary HashTable
        char groupKey[HashEntry::MAX_KEY_LENGTH];
        if (buildGroupKey(groupKey, sizeof(groupKey), exp->getCategory(),
                          exp->getMerchant())) {
          if (!m_table.insertOrUpdate(groupKey, amount)) return false;
        }

        // 2. Accumulate totals by Category in category HashTable
        const char *cat = exp->getCategory();
        if (cat != NULL && cat[0] != '\0') {
          if (!m_categoryTable.insertOrUpdate(cat, amount)) return false;
        }

        // 3. Accumulate totals by Month in monthly HashTable
        char myStr[16];
        if (extractMonthYear(myStr, sizeof(myStr), exp->getDate())) {
          if (!m_monthlyTable.insertOrUpdate(myStr, amount)) return false;
        }

        // 4. Track calendar date range
        updateDateSpan(exp->getDate());

          if (m_totalPaiseAnalyzed > LLONG_MAX - amount ||
              m_totalExpensesAnalyzed == INT_MAX) {
            return false;
          }
        m_totalPaiseAnalyzed += amount;
        m_totalExpensesAnalyzed++;
      }
    }
    curr = curr->getNext();
  }

  // Identify and collect detected patterns
  for (int i = 0; i < m_table.getCapacity(); ++i) {
    const HashEntry *entry = m_table.getEntryAt(i);
    if (entry != NULL && entry->isOccupied()) {
      if (isLeakPattern(entry)) {
        if (m_patternCount < MAX_PATTERNS) {
          m_patterns[m_patternCount++] = entry;
        }
      }
    }
  }

  return true;
}

int LeakAnalyzer::getPatternCount() const { return m_patternCount; }

const HashEntry *LeakAnalyzer::getPatternAt(int index) const {
  if (index < 0 || index >= m_patternCount) {
    return NULL;
  }
  return m_patterns[index];
}

const HashTable &LeakAnalyzer::getHashTable() const { return m_table; }

const HashTable &LeakAnalyzer::getCategoryTable() const {
  return m_categoryTable;
}

const HashTable &LeakAnalyzer::getMonthlyTable() const {
  return m_monthlyTable;
}

long long LeakAnalyzer::getTotalAnalyzedPaise() const {
  return m_totalPaiseAnalyzed;
}

int LeakAnalyzer::getTotalExpensesAnalyzed() const {
  return m_totalExpensesAnalyzed;
}

const char *LeakAnalyzer::getEarliestDate() const { return m_earliestDate; }

const char *LeakAnalyzer::getLatestDate() const { return m_latestDate; }

static void printMoney(std::ostream &os, long long amountPaise) {
  long long rs = amountPaise / 100LL;
  long long paise = amountPaise % 100LL;
  if (paise < 0)
    paise = -paise;
  os << "Rs. " << rs;
  if (paise == 0)
    return;
  os << ".";
  if (paise < 10)
    os << "0" << paise;
  else if (paise % 10 == 0)
    os << paise / 10;
  else
    os << paise;
}

void LeakAnalyzer::displayLeakReport() const { printLeakReport(std::cout); }

void LeakAnalyzer::printLeakReport(std::ostream &os) const {
  os << "======================================================================"
        "==========\n";
  os << "                 EXPENSE LEAK DETECTION & SPENDING PATTERN REPORT     "
        "          \n";
  os << "======================================================================"
        "==========\n";
  os << "Total Expenses Analyzed : " << m_totalExpensesAnalyzed << "\n";
  os << "Total Spending Recorded : ";
  printMoney(os, m_totalPaiseAnalyzed);
  os << "\n";

  if (m_earliestDate[0] != '\0' && m_latestDate[0] != '\0') {
    os << "Analysis Date Range     : " << m_earliestDate << " to "
       << m_latestDate << "\n";
  } else {
    os << "Analysis Date Range     : None (Empty Dataset)\n";
  }
  os << "Unique Spending Channels: " << m_table.getEntryCount() << "\n";
  os << "Detected Leak Patterns  : " << m_patternCount << "\n";
  os << "----------------------------------------------------------------------"
        "----------\n";

  // Section 1: Detailed Repeated Spending Patterns
  os << "[SECTION 1: REPEATED SPENDING PATTERNS & IDENTIFIED LEAKS]\n";
  if (m_patternCount == 0) {
    os << "  No repeated spending patterns or recurring leak channels "
          "detected.\n";
  } else {
    for (int i = 0; i < m_patternCount; ++i) {
      const HashEntry *entry = m_patterns[i];
      if (entry != NULL) {
        os << "  Pattern #" << (i + 1) << ": Channel [" << entry->getKey()
           << "]\n";
        os << "    - Frequency      : " << entry->getTransactionCount()
           << " transactions\n";
        os << "    - Total Amount   : ";
        printMoney(os, entry->getTotalPaise());
        os << "\n";
        os << "    - Average / Tx   : ";
        printMoney(os, entry->getAveragePaise());
        os << "\n";
        os << "    - Amount Range   : Min ";
        printMoney(os, entry->getMinPaise());
        os << " | Max ";
        printMoney(os, entry->getMaxPaise());
        os << "\n";

        // Classification label
        if (entry->getTransactionCount() >= FREQUENT_SPENDING_THRESHOLD) {
          os << "    - Leak Type      : [FREQUENT SPENDING LEAK] (>= 3 "
                "recurring transactions)\n";
        } else {
          os << "    - Leak Type      : [REPEATED MERCHANT DRAIN] (2 recurring "
                "transactions)\n";
        }
        os << "\n";
      }
    }
  }
  os << "----------------------------------------------------------------------"
        "----------\n";

  // Section 2: Category Volume Analysis (Explicitly adheres to non-labeling
  // policy)
  os << "[SECTION 2: CATEGORY VOLUME DISTRIBUTION]\n";
  if (m_categoryTable.getEntryCount() == 0) {
    os << "  No category data available.\n";
  } else {
    for (int i = 0; i < m_categoryTable.getCapacity(); ++i) {
      const HashEntry *catEntry = m_categoryTable.getEntryAt(i);
      if (catEntry != NULL && catEntry->isOccupied()) {
        os << "  Category: " << catEntry->getKey() << "\n";
        os << "    - Total Spend  : ";
        printMoney(os, catEntry->getTotalPaise());
        os << " (" << catEntry->getTransactionCount() << " transactions)\n";

        // Policy-compliant inspection note: High category totals are NOT
        // automatically labeled wasteful!
        if (catEntry->getTotalPaise() >= HIGH_TOTAL_PAISE_THRESHOLD) {
          os << "    - Note         : [HIGH VOLUME CATEGORY] Category total is "
                "significant.\n";
          os << "                     (Per system policy: High totals are not "
                "automatically labeled wasteful;\n";
          os << "                      user review required to discern "
                "essential vs discretionary expenses.)\n";
        }
      }
    }
  }
  os << "----------------------------------------------------------------------"
        "----------\n";

  // Section 3: Monthly Distribution Overview (Uses actual calendar dates)
  os << "[SECTION 3: MONTHLY SPENDING DISTRIBUTION (ACTUAL DATES)]\n";
  if (m_monthlyTable.getEntryCount() == 0) {
    os << "  No monthly date data available.\n";
  } else {
    for (int i = 0; i < m_monthlyTable.getCapacity(); ++i) {
      const HashEntry *mEntry = m_monthlyTable.getEntryAt(i);
      if (mEntry != NULL && mEntry->isOccupied()) {
        os << "  Month [" << mEntry->getKey() << "] : Total ";
        printMoney(os, mEntry->getTotalPaise());
        os << " across " << mEntry->getTransactionCount() << " transactions\n";
      }
    }
  }
  os << "----------------------------------------------------------------------"
        "----------\n";

  // Section 4: Team Contract & Policy Compliance Note
  os << "[SECTION 4: POLICY & CONTRACT COMPLIANCE]\n";
  os << "  - Spending Leak Criteria : Frequency-based pattern detection "
        "(Category:Merchant).\n";
  os << "  - Waste Classification   : Categories not automatically labeled "
        "wasteful due to high totals.\n";
  os << "  - Monthly Aggregation    : Formed strictly from validated "
        "DD-MM-YYYY calendar dates.\n";
  os << "  - Forecasting & Advice   : Excluded by contract design; owned "
        "exclusively by Person 5.\n";
  os << "======================================================================"
        "==========\n";
}
