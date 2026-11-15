#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

/*
 * ============================================================================
 * FILE MANAGER MODULE
 * Owned by: Person 2 (Persistence & Export Module)
 *
 * Responsibilities:
 * - Load and save expense records using the agreed pipe-delimited format.
 * - Preserve saved IDs and insert records via manager.addLoadedExpense().
 * - Reject duplicate IDs and report exact malformed line numbers.
 * - Handle missing and empty files safely without crashing.
 * - Guarantee transactional safety: never silently erase or corrupt in-memory
 *   data if a file load fails.
 * - Write complete snapshots to a temporary file before replacing the target.
 * - Zero STL container dependency.
 * ============================================================================
 */

// Forward declaration of ExpenseManager to maintain strict decoupling
class ExpenseManager;

class FileManager {
public:
  // Constructor accepting target data file path (defaults to "expense.txt")
  FileManager(const char *filePath = "expense.txt");
  ~FileManager();

  // Load records from storage file into the provided ExpenseManager.
  // If loading fails, in-memory dataset remains completely untouched.
  bool loadExpenses(ExpenseManager &manager);

  // Save current ExpenseManager records to storage file via temporary snapshot.
  bool saveExpenses(const ExpenseManager &manager);

  // Return human-readable description of the last encountered error.
  const char *getLastError() const;

  // Path getters/setters
  const char *getFilePath() const;
  void setFilePath(const char *filePath);

private:
  char m_filePath[260];
  char m_lastError[512];

  void setLastError(const char *error);
  void clearLastError();
  static bool ensureParentDirectoryExists(const char *filePath);
};

#endif // FILE_MANAGER_H
