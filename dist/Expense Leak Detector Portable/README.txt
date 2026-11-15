Expense Leak Detector - Portable Windows App

REQUIREMENTS
- 64-bit Windows 10 or later.
- No VS Code, MinGW, GCC, or C++ development tools are required.

LAUNCH
Double-click expense_leak_detector_gui.exe.

DATA AND REPORTS
- Expense records are stored in expense.txt beside the executable.
- The included expense.txt preserves the records copied when this package
  was first created. Keep a backup before replacing or editing it.
- If expense.txt is missing, the app starts with an empty list and creates the
  file when expenses are saved.
- Monthly reports are created beside the executable.
- Keep write access to this folder so the app can save expenses and reports.

PORTABILITY
The GCC and C++ standard-library runtimes are statically linked. The app still
uses standard Windows components (Win32 GUI, common controls, and the Windows
Universal C Runtime), which are provided by supported Windows installations.
No MinGW runtime DLLs are included or required.

To test a copy, extract the package to a writable folder, launch the app, add
an expense, confirm it appears in the list, close the app, then reopen it and
confirm the expense is still present. The app writes changes to the local
expense.txt file.
