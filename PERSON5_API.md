# Person 5 API Documentation (Phase 2 - 40% Milestone)

## Overview
This package contains the core data structures and financial advisement logic for Person 5.

## Files Included:
1. `UndoAction.h` & `UndoAction.cpp`: Manages action states and snapshots for reversible expense operations.
2. `UndoStack.h` & `UndoStack.cpp`: Fixed-capacity array-based stack implementation for undo operations without STL containers.
3. `SavingsAdvisor.h` & `SavingsAdvisor.cpp`: Analyzes expenses, calculates monthly totals, projects run-rate forecasts, and evaluates budget targets.
4. `person5_test.cpp`: Unit tests validating UndoStack boundaries, overflow/underflow handling, and push/pop sequencing.
