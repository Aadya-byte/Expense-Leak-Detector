#include "UndoStack.h"

UndoStack::UndoStack() : topIndex(-1) {}

bool UndoStack::push(const UndoAction& action) {
    if (isFull()) return false;
    data[++topIndex] = action;
    return true;
}

bool UndoStack::pop(UndoAction& action) {
    if (isEmpty()) return false;
    action = data[topIndex--];
    return true;
}

bool UndoStack::peek(UndoAction& action) const {
    if (isEmpty()) return false;
    action = data[topIndex];
    return true;
}

bool UndoStack::isEmpty() const { return topIndex == -1; }
bool UndoStack::isFull() const { return topIndex >= MAX_CAPACITY - 1; }
void UndoStack::clear() { topIndex = -1; }
int UndoStack::getSize() const { return topIndex + 1; }
