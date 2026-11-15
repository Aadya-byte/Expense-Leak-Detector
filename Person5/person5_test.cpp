#include <iostream>
#include <cassert>
#include "UndoStack.h"
#include "UndoAction.h"
#include "SavingsAdvisor.h"

void testUndoStack() {
    std::cout << "--> Running UndoStack Unit Tests...
";
    UndoStack stack;
    
    assert(stack.isEmpty() == true);
    assert(stack.getSize() == 0);

    for (int i = 0; i < 100; ++i) {
        UndoAction action(ActionType::ADD, i);
        assert(stack.push(action) == true);
    }
    
    assert(stack.isFull() == true);
    assert(stack.getSize() == 100);
    
    UndoAction overflowAction(ActionType::DELETE, 999);
    assert(stack.push(overflowAction) == false);

    UndoAction popped;
    for (int i = 99; i >= 0; --i) {
        assert(stack.pop(popped) == true);
        assert(popped.getExpenseId() == i);
    }

    assert(stack.isEmpty() == true);
    std::cout << "SUCCESS: UndoStack tests passed.
";
}

int main() {
    testUndoStack();
    std::cout << "ALL PERSON 5 TESTS PASSED SUCCESSFULLY!
";
    return 0;
}
