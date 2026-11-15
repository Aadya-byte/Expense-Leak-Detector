# Expense Leak Detector — Integrated Build

## Build and run the Windows GUI

Install MinGW-w64 with `g++` available on `PATH`, then run this from any
PowerShell location:

```powershell
& 'C:\path\to\project\Person6\build_gui.ps1'
```

The script builds `expense_leak_detector_gui.exe` in the project root and
launches it. The GUI uses the same `expense.txt` snapshot as the console app.
The supplied GUI source and its adapter are `GUI.cpp` and `GuiLogic.cpp`; the
backend modules remain shared and unchanged.

To build without immediately launching the GUI, run the same compiler command
in the script and omit the final `& .\expense_leak_detector_gui.exe` line.

The application uses Person 1's `Expense` and `ExpenseManager` as the shared
model. Person 2's persistence module stores a plain-text, pipe-delimited
snapshot in `expense.txt` beside the application executable; no database,
JSON, or SQL is used. Monthly reports are written as
`monthly_report_MM-YYYY.txt` beside the expense file.

Person 2 and Person 3 also contain alternate `Expense.h` / `ExpenseManager.h`
contracts and test adapters. Do not compile those alternate headers or test
adapters into the application. The integrated sources use Person 1's model.

## Build the application (from the project root)

```powershell
g++ -std=c++11 -Wall -Wextra -pedantic `
  -IPerson1 -IPerson2 -IPerson3 -IPerson4 -IPerson5 -IPerson6 `
  Person1\Expense.cpp Person1\ExpenseNode.cpp Person1\ExpenseManager.cpp `
  Person2\FileManager.cpp Person2\RecordParser.cpp Person2\ReportExporter.cpp `
  Person3\HashEntry.cpp Person3\HashTable.cpp Person3\LeakAnalyzer.cpp `
  Person4\Search.cpp Person4\Sort.cpp Person4\BSTNode.cpp Person4\ExpenseBST.cpp `
  Person5\SavingsAdvisor.cpp Person5\UndoAction.cpp Person5\UndoStack.cpp `
  Person6\UI.cpp Person6\main.cpp -o expense_leak_detector_chart_grouped.exe
```

## Build and run the integrated tests

Compile the same module sources above, adding `Person6\GuiLogic.cpp` and
`Person6\UI.cpp`, replacing `Person6\main.cpp` with
`Person6\IntegrationTest.cpp`, and using `-o integration_test`. Do not include
both `main.cpp` and `IntegrationTest.cpp` because each contains its own `main()`.

The integration test covers the shared manager, search/sort/BST, undo stack,
record parser, plain-text save/load, leak analysis, monthly report export, and
integer-boundary cases.

## Runtime behavior

- Amounts are entered and stored as integer paise.
- Add, edit, delete, and undo save the current snapshot immediately; option 11
  also saves before exiting.
- On Windows, both the expense file and monthly reports are stored beside the
  application executable, regardless of the current working folder.
- On the first run, a missing snapshot is treated as an empty expense list.
- Option 10 exports the selected month's transactions, spending-leak analysis,
  and forecast, excluding expenses from other months.
- Sorted results are grouped under category headings, with each merchant
  listed in its own aligned table row beneath the category.
- Reported currency values omit trailing zeroes while retaining meaningful
  paise (for example, `Rs. 100`, `Rs. 12.5`, and `Rs. 12.05`).
- Expense records are owned by `ExpenseManager`; search/sort pointer arrays
  are temporary non-owning views.
