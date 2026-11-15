# Person 4 — Full Available Source Bundle

This ZIP collects the Person 4 source files and documentation currently available in the conversation workspace.

## Included
- Search.h / Search.cpp
- Sort.h / Sort.cpp
- BSTNode.h / BSTNode.cpp
- ExpenseBST.h / ExpenseBST.cpp
- person4_test.cpp
- PERSON4_API.md

## Important integration note
This is the full set of Person 4 files available here, but it cannot honestly be certified as a complete, standalone 100% application build. The modules and test depend on shared Person 1 files, including Expense.h, ExpenseTypes.h, and ExpenseManager.h/.cpp, which are not included in this Person 4 bundle. Add the team's agreed shared files when integrating.

The source files originally arrived as Search1.cpp and SORT1.cpp; this package uses conventional names Search.cpp and Sort.cpp to match the headers and API documentation. The implementation content is preserved.

Before submitting as 100% complete:
1. Compile with the real shared Person 1 files.
2. Run person4_test.cpp against the integrated project.
3. Fix any interface/build issues and record actual test results.
4. Confirm the integrated branch contains the final versions.
