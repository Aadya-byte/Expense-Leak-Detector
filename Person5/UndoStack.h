#ifndef UNDO_STACK_H
#define UNDO_STACK_H

#include "UndoAction.h"

class UndoStack {
private:
    static const int MAX_CAPACITY = 100;
    UndoAction data[MAX_CAPACITY];
    int topIndex;

public:
    UndoStack();
    
    bool push(const UndoAction& action);
    bool pop(UndoAction& action);
    bool peek(UndoAction& action) const;
    
    bool isEmpty() const;
    bool isFull() const;
    void clear();
    int getSize() const;
};

#endif // UNDO_STACK_H
