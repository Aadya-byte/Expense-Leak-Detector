#include "UI.h"
#include "Expense.h"
#include "FileManager.h"
#include "ReportExporter.h"
#include "UndoAction.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <limits>
#include <new>
#include <cstdio>
#include <cstdlib>
#include <cstring>

UI::UI(ExpenseManager& m, FileManager& fm, LeakAnalyzer& la, Search& s,
       Sort& so, ExpenseBST& bst, SavingsAdvisor& sa, UndoStack& us)
    : manager(m), fileManager(fm), leakAnalyzer(la), search(s), sort(so),
      expenseBST(bst), savingsAdvisor(sa), undoStack(us) {}

static void printMoney(std::ostream& out, long long amountPaise) {
    out << "Rs. " << amountPaise / 100;
    const int paise = static_cast<int>(amountPaise % 100);
    if (paise == 0) return;
    out << '.';
    if (paise < 10) out << '0' << paise;
    else if (paise % 10 == 0) out << paise / 10;
    else out << paise;
}

static void printMoney(long long amountPaise) {
    printMoney(std::cout, amountPaise);
}

static void printTextCell(std::ostream& out, const char* text, int width) {
    const int length = text ? static_cast<int>(std::strlen(text)) : 0;
    if (length <= width) {
        if (length > 0) out.write(text, length);
        for (int i = length; i < width; ++i) out << ' ';
        return;
    }
    if (width <= 3) {
        out.write(text, width);
        return;
    }
    out.write(text, width - 3);
    out << "...";
}

static void printExpenseTableRule(std::ostream& out) {
    const int widths[] = {6, 10, 12, 18, 24, 12, 10};
    for (int column = 0; column < 7; ++column) {
        for (int i = 0; i < widths[column]; ++i) out << '-';
        if (column < 6) out << "-+-";
    }
    out << "\n";
}

static void printExpenseTable(std::ostream& out, Expense* const items[],
                              int count, const char* title) {
    out << "\n" << title << "\n";
    printExpenseTableRule(out);
    printTextCell(out, "ID", 6); out << " | ";
    printTextCell(out, "Date", 10); out << " | ";
    printTextCell(out, "Amount", 12); out << " | ";
    printTextCell(out, "Category", 18); out << " | ";
    printTextCell(out, "Merchant", 24); out << " | ";
    printTextCell(out, "Type", 12); out << " | ";
    printTextCell(out, "Frequency", 10); out << "\n";
    printExpenseTableRule(out);

    for (int i = 0; i < count; ++i) {
        if (!items[i]) continue;
        std::ostringstream amount;
        printMoney(amount, items[i]->getAmountPaise());
        char id[16];
        std::snprintf(id, sizeof(id), "%d", items[i]->getId());

        printTextCell(out, id, 6); out << " | ";
        printTextCell(out, items[i]->getDate(), 10);
        out << " | ";
        printTextCell(out, amount.str().c_str(), 12);
        out << " | ";
        printTextCell(out, items[i]->getCategory(), 18);
        out << " | ";
        printTextCell(out, items[i]->getMerchant(), 24);
        out << " | ";
        printTextCell(out, items[i]->getExpenseType(), 12);
        out << " | ";
        printTextCell(out, items[i]->getFrequency(), 10);
        out << "\n";
    }
    printExpenseTableRule(out);
    out << count << (count == 1 ? " expense\n" : " expenses\n");
}

static void printCategoryGroupedExpenseTable(std::ostream& out,
                                             Expense* const items[],
                                             int count, const char* title) {
    out << "\n" << title << "\n";
    bool printedCategory = false;
    for (int i = 0; i < count; ++i) {
        if (!items[i]) continue;

        const char* category = items[i]->getCategory();
        bool alreadyPrinted = false;
        for (int previous = 0; previous < i; ++previous) {
            if (items[previous] &&
                std::strcmp(items[previous]->getCategory(), category) == 0) {
                alreadyPrinted = true;
                break;
            }
        }
        if (alreadyPrinted) continue;

        if (printedCategory) out << "\n";
        out << "CATEGORY: " << category << "\n";
        const int widths[] = {6, 10, 12, 24, 12, 10};
        for (int column = 0; column < 6; ++column) {
            for (int dash = 0; dash < widths[column]; ++dash) out << '-';
            if (column < 5) out << "-+-";
        }
        out << "\n";
        printTextCell(out, "ID", widths[0]); out << " | ";
        printTextCell(out, "Date", widths[1]); out << " | ";
        printTextCell(out, "Amount", widths[2]); out << " | ";
        printTextCell(out, "Merchant", widths[3]); out << " | ";
        printTextCell(out, "Type", widths[4]); out << " | ";
        printTextCell(out, "Frequency", widths[5]); out << "\n";
        for (int column = 0; column < 6; ++column) {
            for (int dash = 0; dash < widths[column]; ++dash) out << '-';
            if (column < 5) out << "-+-";
        }
        out << "\n";

        for (int row = 0; row < count; ++row) {
            if (!items[row] ||
                std::strcmp(items[row]->getCategory(), category) != 0) continue;

            std::ostringstream amount;
            printMoney(amount, items[row]->getAmountPaise());
            char id[16];
            std::snprintf(id, sizeof(id), "%d", items[row]->getId());
            printTextCell(out, id, widths[0]); out << " | ";
            printTextCell(out, items[row]->getDate(), widths[1]); out << " | ";
            printTextCell(out, amount.str().c_str(), widths[2]); out << " | ";
            printTextCell(out, items[row]->getMerchant(), widths[3]); out << " | ";
            printTextCell(out, items[row]->getExpenseType(), widths[4]); out << " | ";
            printTextCell(out, items[row]->getFrequency(), widths[5]); out << "\n";
        }

        for (int column = 0; column < 6; ++column) {
            for (int dash = 0; dash < widths[column]; ++dash) out << '-';
            if (column < 5) out << "-+-";
        }
        out << "\n";
        printedCategory = true;
    }
    out << count << (count == 1 ? " expense\n" : " expenses\n");
}

static bool matchesMonthYear(const char* date, int month, int year) {
    if (!date || std::strlen(date) != 10 || date[2] != '-' || date[5] != '-')
        return false;
    for (int i = 0; i < 10; ++i) {
        if (i == 2 || i == 5) continue;
        if (date[i] < '0' || date[i] > '9') return false;
    }
    const int dateMonth = (date[3] - '0') * 10 + date[4] - '0';
    const int dateYear = (date[6] - '0') * 1000 + (date[7] - '0') * 100 +
                         (date[8] - '0') * 10 + date[9] - '0';
    return dateMonth == month && dateYear == year;
}

bool UI::readInt(const char* prompt, int& value) const {
    std::cout << prompt;
    if (std::cin >> value) return true;
    std::cin.clear();
    std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
    std::cout << "Invalid input. Please enter a whole number.\n";
    return false;
}

bool UI::readPositiveAmount(const char* prompt, long long& value) const {
    std::cout << prompt;
    if (!(std::cin >> value) || value <= 0) {
        std::cin.clear();
        std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
        std::cout << "Amount must be a positive integer in paise.\n";
        return false;
    }
    return true;
}

bool UI::readText(const char* prompt, char* buffer, int capacity) const {
    if (!buffer || capacity < 2) return false;
    std::cout << prompt;
    std::cin >> std::ws;
    std::cin.getline(buffer, capacity);
    if (std::cin.fail()) {
        std::cin.clear();
        std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
        std::cout << "Input too long. Please try again with a shorter value.\n";
        return false;
    }
    if (buffer[0] == '\0') {
        std::cout << "Input cannot be empty.\n";
        return false;
    }
    return true;
}

void UI::displayMainMenu() const {
    std::cout << "\n========== EXPENSE LEAK DETECTOR ==========\n"
              << "1. Add expense\n"
              << "2. View all expenses\n"
              << "3. Edit expense\n"
              << "4. Delete expense\n"
              << "5. Search expense\n"
              << "6. Sort expenses / amount index\n"
              << "7. View spending leak report\n"
              << "8. View monthly forecast and savings\n"
              << "9. Undo last supported operation\n"
              << "10. Export monthly report\n"
              << "11. Save and exit\n"
              << "===========================================\n";
}

void UI::addExpense() {
    char date[16], merchant[64], category[40], type[20], frequency[20];
    long long amount;
    int id = 0;
    if (!readText("Date (DD-MM-YYYY): ", date, sizeof(date)) ||
        !readPositiveAmount("Amount in paise (e.g. 12550 = Rs.125.50): ", amount) ||
        !readText("Merchant: ", merchant, sizeof(merchant)) ||
        !readText("Category: ", category, sizeof(category)) ||
        !readText("Type (ONE_TIME/RECURRING): ", type, sizeof(type)) ||
        !readText("Frequency (NONE/DAILY/WEEKLY/MONTHLY): ", frequency, sizeof(frequency)))
        return;

    if (!manager.addExpense(date, amount, merchant, category, type, frequency, id)) {
        std::cout << "Error: expense was not added. Check date, amount, field lengths, "
                     "type/frequency combination, and duplicate/ID constraints.\n";
        return;
    }
    if (!fileManager.saveExpenses(manager))
        std::cout << "Warning: expense was added but could not be saved: "
                  << fileManager.getLastError() << "\n";
    const Expense* saved = manager.findExpenseById(id);
    if (saved) {
        UndoAction action(ActionType::ADD, id);
        action.setAfterSnapshot(*saved);
        if (!undoStack.push(action))
            std::cout << "Warning: expense added, but undo history is full.\n";
    }
    std::cout << "Expense added successfully. Assigned ID: " << id << "\n";
}

void UI::viewExpenses() const {
    if (manager.getCount() == 0) {
        std::cout << "No expenses recorded yet.\n";
        return;
    }
    std::cout << "\nID   DATE        AMOUNT (Rs.)    MERCHANT                     CATEGORY\n";
    std::cout << "--------------------------------------------------------------------------\n";
    const ExpenseNode* node = manager.getHead();
    while (node) {
        // Person 1's ExpenseNode stores an Expense by value.
        const Expense& e = node->getExpense();
        std::cout << e.getId() << "   " << e.getDate() << "   ";
        printMoney(e.getAmountPaise());
        std::cout << "       " << e.getMerchant()
                  << "   " << e.getCategory() << "\n";
        node = node->getNext();
    }
}

void UI::editExpense() {
    int id;
    if (!readInt("Enter expense ID to edit: ", id)) return;
    const Expense* oldPtr = manager.findExpenseById(id);
    if (!oldPtr) { std::cout << "No expense found with that ID.\n"; return; }
    Expense before(*oldPtr);
    char date[16], merchant[64], category[40], type[20], frequency[20];
    long long amount;
    if (!readText("New date (DD-MM-YYYY): ", date, sizeof(date)) ||
        !readPositiveAmount("New amount in paise: ", amount) ||
        !readText("New merchant: ", merchant, sizeof(merchant)) ||
        !readText("New category: ", category, sizeof(category)) ||
        !readText("New type (ONE_TIME/RECURRING): ", type, sizeof(type)) ||
        !readText("New frequency (NONE/DAILY/WEEKLY/MONTHLY): ", frequency, sizeof(frequency)))
        return;
    Expense updated(id, date, amount, merchant, category, type, frequency);
    if (!manager.editExpense(id, updated)) {
        std::cout << "Edit failed. Check the entered values.\n";
        return;
    }
    if (!fileManager.saveExpenses(manager))
        std::cout << "Warning: edit was applied but could not be saved: "
                  << fileManager.getLastError() << "\n";
    const Expense* after = manager.findExpenseById(id);
    if (after) {
        UndoAction action(ActionType::EDIT, id, before, *after);
        if (!undoStack.push(action))
            std::cout << "Warning: edit saved, but undo history is full.\n";
    }
    std::cout << "Expense updated successfully.\n";
}

void UI::deleteExpense() {
    int id;
    if (!readInt("Enter expense ID to delete: ", id)) return;
    const Expense* found = manager.findExpenseById(id);
    if (!found) { std::cout << "No expense found with that ID.\n"; return; }
    Expense before(*found);
    if (!manager.deleteExpense(id)) {
        std::cout << "Delete failed.\n";
        return;
    }
    if (!fileManager.saveExpenses(manager))
        std::cout << "Warning: expense was deleted but could not be saved: "
                  << fileManager.getLastError() << "\n";
    UndoAction action(ActionType::DELETE, id);
    action.setBeforeSnapshot(before);
    if (!undoStack.push(action))
        std::cout << "Warning: expense deleted, but undo history is full.\n";
    std::cout << "Expense deleted successfully.\n";
}

void UI::searchExpense() {
    int id;
    if (!readInt("Enter expense ID to search: ", id)) return;
    int count = manager.getCount();
    if (count <= 0) { std::cout << "No expenses to search.\n"; return; }
    Expense** items = new (std::nothrow) Expense*[count];
    if (!items) { std::cout << "Not enough memory to search.\n"; return; }
    int copied = manager.copyExpensePointers(items, count);
    if (copied < 0) {
        delete[] items;
        std::cout << "Could not prepare the search index.\n";
        return;
    }
    Expense* result = search.linearSearchById(items, copied, id);
    if (result) {
        Expense* resultItems[1];
        resultItems[0] = result;
        printExpenseTable(std::cout, resultItems, 1, "SEARCH RESULT");
    } else std::cout << "No expense found with that ID.\n";
    delete[] items; // Only the pointer array is owned here, not the Expense records.
}

void UI::sortExpenses() {
    int choice;
    if (!readInt("Sort by: 1=ID, 2=Amount, 3=Date, 4=Amount index: ", choice)) return;
    int count = manager.getCount();
    if (count <= 0) { std::cout << "No expenses to sort.\n"; return; }
    Expense** items = new (std::nothrow) Expense*[count];
    if (!items) { std::cout << "Not enough memory to sort.\n"; return; }
    int copied = manager.copyExpensePointers(items, count);
    if (copied < 0) { delete[] items; std::cout << "Could not prepare sort data.\n"; return; }
    if (choice == 1) sort.sortById(items, copied);
    else if (choice == 2) sort.sortByAmount(items, copied);
    else if (choice == 3) sort.sortByDate(items, copied);
    else if (choice == 4) {
        expenseBST.clear();
        bool indexed = true;
        for (int i = 0; i < copied; ++i) {
            if (!expenseBST.insert(*items[i])) {
                indexed = false;
                break;
            }
        }
        if (!indexed) {
            delete[] items;
            std::cout << "Could not build the amount index.\n";
            return;
        }
        sort.sortByAmount(items, copied);
    }
    else { delete[] items; std::cout << "Invalid sort choice.\n"; return; }
    const char* title = choice == 1 ? "EXPENSES SORTED BY ID" :
                        choice == 2 ? "EXPENSES SORTED BY AMOUNT" :
                        choice == 3 ? "EXPENSES SORTED BY DATE" :
                                      "EXPENSES SORTED BY AMOUNT INDEX";
    printCategoryGroupedExpenseTable(std::cout, items, copied, title);
    delete[] items;
}

void UI::showLeakReport() {
    if (!leakAnalyzer.analyzeExpenses(manager)) {
        std::cout << "Leak analysis failed because the data exceeds analyzer limits.\n";
        return;
    }
    leakAnalyzer.displayLeakReport();
}

void UI::showForecast() {
    int month, year, reduction;
    if (!readInt("Month (1-12): ", month) ||
        !readInt("Year (1900-2100): ", year)) return;
    if (month < 1 || month > 12 || year < 1900 || year > 2100) {
        std::cout << "Invalid month or year.\n";
        return;
    }
    long long total = savingsAdvisor.calculateMonthlyTotal(manager, month, year);
    long long forecast = savingsAdvisor.calculateMonthlyForecast(manager, month, year);
    if (!readInt("Potential savings reduction percentage (0-100): ", reduction)) return;
    if (reduction < 0 || reduction > 100) {
        std::cout << "Percentage must be from 0 to 100.\n";
        return;
    }
    long long savings = savingsAdvisor.calculatePotentialSavings(manager, month, year, reduction);
    std::cout << "Monthly total: ";
    printMoney(total);
    std::cout << "\nForecast: ";
    printMoney(forecast);
    std::cout << "\nPotential savings at " << reduction << "%: ";
    printMoney(savings);
    std::cout << "\n";
}

void UI::undoLast() {
    UndoAction action;
    if (!undoStack.pop(action)) { std::cout << "Nothing to undo.\n"; return; }
    const int id = static_cast<int>(action.getExpenseId());
    bool ok = false;
    if (action.getType() == ActionType::ADD) {
        ok = manager.deleteExpense(id);
    } else if (action.getType() == ActionType::DELETE) {
        Expense before;
        if (action.getBeforeSnapshot(before)) ok = manager.addLoadedExpense(before);
    } else if (action.getType() == ActionType::EDIT) {
        Expense before;
        if (action.getBeforeSnapshot(before)) ok = manager.editExpense(id, before);
    }
    if (ok) {
        if (!fileManager.saveExpenses(manager))
            std::cout << "Undo completed, but could not save the result: "
                      << fileManager.getLastError() << "\n";
        else
            std::cout << "Undo completed and saved.\n";
    }
    else {
        std::cout << "Undo failed; action was returned to the stack if possible.\n";
        undoStack.push(action);
    }
}

void UI::exportMonthlyReport() {
    int month, year;
    if (!readInt("Month (1-12): ", month) ||
        !readInt("Year (1-9999): ", year)) return;
    if (month < 1 || month > 12 || year < 1 || year > 9999) {
        std::cout << "Invalid month or year.\n";
        return;
    }

    char reportName[64];
    std::snprintf(reportName, sizeof(reportName),
                  "monthly_report_%02d-%04d.txt", month, year);
    const char* dataPath = fileManager.getFilePath();
    const char* pathSeparator = std::strrchr(dataPath, '\\');
    const char* forwardSlash = std::strrchr(dataPath, '/');
    if (forwardSlash != NULL &&
        (pathSeparator == NULL || forwardSlash > pathSeparator))
        pathSeparator = forwardSlash;
    const std::size_t directoryLength =
        pathSeparator == NULL ? 0 : static_cast<std::size_t>(pathSeparator - dataPath + 1);
    const std::size_t reportNameLength = std::strlen(reportName);
    char outputPath[260];
    if (directoryLength + reportNameLength >= sizeof(outputPath)) {
        std::cout << "Report path is too long.\n";
        return;
    }
    std::memcpy(outputPath, dataPath, directoryLength);
    std::memcpy(outputPath + directoryLength, reportName, reportNameLength + 1);

    ReportExporter exporter;
    if (!exporter.exportMonthlyReport(manager, month, year, outputPath)) {
        std::cout << "Report export failed for " << outputPath << ".\n";
        return;
    }

    ExpenseManager monthlyManager;
    const ExpenseNode* node = manager.getHead();
    while (node) {
        const Expense& expense = node->getExpense();
        if (matchesMonthYear(expense.getDate(), month, year) &&
            !monthlyManager.addLoadedExpense(expense)) {
            std::cout << "Report was exported, but its monthly analysis could not "
                         "include every matching expense.\n";
            return;
        }
        node = node->getNext();
    }

    std::ofstream report(outputPath, std::ios::out | std::ios::app);
    if (!report.is_open()) {
        std::cout << "Report was exported, but its analysis sections could not be appended.\n";
        return;
    }
    report << "\nSPENDING LEAK REPORT - " << month << "/" << year << "\n";
    LeakAnalyzer monthlyLeakAnalyzer;
    if (!monthlyLeakAnalyzer.analyzeExpenses(monthlyManager)) {
        report.close();
        std::cout << "Report was exported, but monthly leak analysis failed.\n";
        return;
    }
    monthlyLeakAnalyzer.printLeakReport(report);

    report << "\nMONTHLY FORECAST\n";
    if (year < 1900 || year > 2100) {
        report << "Forecast unavailable outside the supported years 1900-2100.\n";
    } else {
        const long long monthlyTotal =
            savingsAdvisor.calculateMonthlyTotal(monthlyManager, month, year);
        const long long monthlyForecast =
            savingsAdvisor.calculateMonthlyForecast(monthlyManager, month, year);
        report << "Actual spending: ";
        printMoney(report, monthlyTotal);
        report << "\nProjected month-end spending: ";
        printMoney(report, monthlyForecast);
        report << "\n";
    }
    report.close();
    if (report.fail()) {
        std::cout << "Report analysis was generated but could not be fully written.\n";
        return;
    }
    std::cout << "Monthly report, leak analysis, and forecast exported to "
              << outputPath << ".\n";
}

void UI::run() {
    bool running = true;
    while (running) {
        displayMainMenu();
        int choice;
        if (!readInt("Select an option (1-11): ", choice)) {
            if (std::cin.eof()) break;
            continue;
        }
        switch (choice) {
            case 1: addExpense(); break;
            case 2: viewExpenses(); break;
            case 3: editExpense(); break;
            case 4: deleteExpense(); break;
            case 5: searchExpense(); break;
            case 6: sortExpenses(); break;
            case 7: showLeakReport(); break;
            case 8: showForecast(); break;
            case 9: undoLast(); break;
            case 10: exportMonthlyReport(); break;
            case 11: running = false; break;
            default: std::cout << "Choose a number from 1 to 11.\n"; break;
        }
    }
}
