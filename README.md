# Expense Leak Detector

A C++ application designed to help users track expenses, analyse spending patterns, and identify potential areas of unnecessary expenditure.

The project combines **Data Structures and Algorithms (DSA)** with **Object-Oriented Programming (OOP)** concepts to provide expense management, spending analysis, and a graphical user interface.

## Features

- **Expense Management:** Add, view, and manage expense records.
- **File Handling:** Save and load expense data for persistence.
- **Spending Analysis:** Calculate category-wise totals, expense frequency, and spending patterns.
- **Leak Detection:** Identify categories and patterns that may indicate unnecessary spending.
- **Search and Sorting:** Organise and retrieve expense records efficiently.
- **Savings Advisor:** Provide suggestions based on spending information.
- **Undo Functionality:** Reverse supported actions.
- **Graphical User Interface:** Interact with the application through a GUI.

## Technologies Used

- C++
- Object-Oriented Programming
- Data Structures and Algorithms
- File Handling
- Windows PowerShell (build script)

## Data Structures and Concepts

The project incorporates several data structures and programming concepts:

| Data Structure / Concept | Purpose |
|---|---|
| Linked List | Managing expense records |
| Hash Table | Organising data for spending analysis |
| Binary Search Tree | Supporting search operations |
| Stack | Implementing undo functionality |
| Queue | Managing queued operations or alerts |
| File Handling | Persistent storage of expense records |
| OOP | Modular design, encapsulation, and code organisation |

## Project Structure

```text
Expense-Leak-Detector/
│
├── Person1/          # Expense management
├── Person2/          # File handling and reports
├── Person3/          # Hash table and leak analysis
├── Person4/          # BST, searching and sorting
├── Person5/          # Savings advisor and undo
├── Person6/          # GUI and application integration
│
├── data/             # Expense data
├── dist/             # Distribution/build files
│
└── README.md
```

## Team Contributions

The project was developed collaboratively, with responsibilities divided across six modules.

| Member | Contribution |
|---|---|
| Person 1 | Expense management and linked-list implementation |
| Person 2 | File management, parsing, and report generation |
| Person 3 | Hash table implementation and expense analysis |
| Person 4 | Binary search tree, searching, and sorting |
| Person 5 | Savings advisor and undo functionality |
| Person 6 | GUI development and application integration |

## Running the Project

The project includes a Windows PowerShell build script in the `Person6` directory.

1. Install a C++ compiler such as MinGW-w64.
2. Open the project directory in Windows.
3. Open PowerShell in the project folder.
4. Run the build script:

```powershell
.\Person6\build_gui.ps1
```

Follow the script's output for build status and execution instructions.

## Learning Outcomes

This project provided practical experience with:

- Implementing and integrating different data structures.
- Applying OOP principles in a modular application.
- Working with file input/output and persistent data.
- Collaborating through separate modules.
- Integrating independently developed components into one application.

## Future Improvements

- More advanced spending-pattern analysis.
- Improved visualisation of expense statistics.
- Additional custom budgeting options.
- Cross-platform application support.

---

**Expense Leak Detector** — A collaborative C++ project focused on applying core programming concepts to a practical expense-management problem.
