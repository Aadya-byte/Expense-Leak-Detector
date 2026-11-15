#include "BSTNode.h"

BSTNode::BSTNode()
    : expenseId(0), amountPaise(0), left(nullptr), right(nullptr) {}

BSTNode::BSTNode(int id, long long amount)
    : expenseId(id), amountPaise(amount), left(nullptr), right(nullptr) {}

BSTNode::~BSTNode() {
    // Note: Child node deletion is handled recursively by ExpenseBST::clear()
}

int BSTNode::getExpenseId() const {
    return expenseId;
}

long long BSTNode::getAmountPaise() const {
    return amountPaise;
}

BSTNode* BSTNode::getLeft() const {
    return left;
}

BSTNode* BSTNode::getRight() const {
    return right;
}

void BSTNode::setLeft(BSTNode* newLeft) {
    left = newLeft;
}

void BSTNode::setRight(BSTNode* newRight) {
    right = newRight;
}

void BSTNode::setExpenseId(int id) {
    expenseId = id;
}

void BSTNode::setAmountPaise(long long amount) {
    amountPaise = amount;
}
