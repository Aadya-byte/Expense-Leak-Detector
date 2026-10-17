#include <iostream>
#include "UndoStack.h"
#include "UndoAction.h"
#include "SavingsAdvisor.h"
#include "ExpenseManager.h"

void testUndoStack() {
    std::cout << "Running UndoStack tests...\n";
    UndoStack stack;
    
    if (!stack.isEmpty()) std::cerr << "FAIL: Stack should be empty\n";
    if (stack.getSize() != 0) std::cerr << "FAIL: Initial size should be 0\n";

    for (int i = 0; i < 100; ++i) {
        UndoAction action(ActionType::ADD, i);
        if (!stack.push(action)) {
            std::cerr << "FAIL: Push failed at index " << i << "\n";
        }
    }
    
    if (!stack.isFull()) std::cerr << "FAIL: Stack should be full\n";
    
    UndoAction extraAction(ActionType::DELETE, 999);
    if (stack.push(extraAction)) {
        std::cerr << "FAIL: Stack allowed push when full (overflow check)\n";
    }

    UndoAction popped;
    for (int i = 99; i >= 0; --i) {
        if (!stack.pop(popped) || popped.getExpenseId() != i) {
            std::cerr << "FAIL: Pop order incorrect at ID " << i << "\n";
        }
    }

    if (!stack.isEmpty()) std::cerr << "FAIL: Stack should be empty after popping all\n";
    std::cout << "UndoStack tests completed.\n";
}

int main() {
    testUndoStack();
    std::cout << "All Person 5 tests executed successfully!\n";
    return 0;
}
