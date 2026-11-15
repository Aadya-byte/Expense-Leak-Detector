#ifndef REPORT_EXPORTER_H
#define REPORT_EXPORTER_H

/*
 * ============================================================================
 * REPORT EXPORTER MODULE
 * Owned by: Person 2 (Persistence & Export Module)
 *
 * Responsibilities:
 * - Generate and export formatted monthly expense reports to text files.
 * - Filter transactions for the requested month and year.
 * - Calculate total spending and transaction count using exact integer paise
 *   arithmetic internally (avoiding floating-point inaccuracies).
 * - Format currency to rupees only for human display (Rs. XX.YY).
 * - Traverse ExpenseManager directly via getHead() and ExpenseNode getters.
 * - Zero STL container dependency.
 * ============================================================================
 */

#include <cstddef>

// Forward declaration of ExpenseManager to maintain strict decoupling
class ExpenseManager;

class ReportExporter {
public:
    ReportExporter();
    ~ReportExporter();

    // Export monthly transaction list, count, and total spending to outputPath.
    // Returns true on success; false on failure (invalid args, write error, etc.)
    bool exportMonthlyReport(const ExpenseManager& manager,
                             int month, int year,
                             const char* outputPath) const;

private:
    static const char* getMonthName(int month);
    static bool parseDateMonthYear(const char* dateStr, int& outMonth, int& outYear);
    static bool ensureParentDirectoryExists(const char* filePath);
};

#endif // REPORT_EXPORTER_H
