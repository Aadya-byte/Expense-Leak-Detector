#include "ExpenseBST.h"
#include <iostream>
#include <new>

ExpenseBST::ExpenseBST()
    : root(nullptr), nodeCount(0) {}

ExpenseBST::ExpenseBST(const ExpenseBST& other)
    : root(nullptr), nodeCount(0) {
    root = copyTreeHelper(other.root);
    nodeCount = other.nodeCount;
}

ExpenseBST& ExpenseBST::operator=(const ExpenseBST& other) {
    if (this != &other) {
        clear();
        root = copyTreeHelper(other.root);
        nodeCount = other.nodeCount;
    }
    return *this;
}

ExpenseBST::~ExpenseBST() {
    clear();
}

BSTNode* ExpenseBST::copyTreeHelper(const BSTNode* otherNode) {
    if (!otherNode) return nullptr;
    BSTNode* newNode = new (std::nothrow) BSTNode(otherNode->getExpenseId(), otherNode->getAmountPaise());
    if (!newNode) return nullptr;
    newNode->left = copyTreeHelper(otherNode->getLeft());
    newNode->right = copyTreeHelper(otherNode->getRight());
    return newNode;
}

bool ExpenseBST::insertHelper(BSTNode*& current, int id, long long amount) {
    if (!current) {
        current = new (std::nothrow) BSTNode(id, amount);
        if (!current) return false;
        nodeCount++;
        return true;
    }

    long long curAmt = current->getAmountPaise();
    int curId = current->getExpenseId();

    // Composite key comparison: (amount, id)
    if (amount < curAmt || (amount == curAmt && id < curId)) {
        return insertHelper(current->left, id, amount);
    } else if (amount > curAmt || (amount == curAmt && id > curId)) {
        return insertHelper(current->right, id, amount);
    } else {
        // Exact duplicate key (same amount and same expense ID)
        return false;
    }
}

bool ExpenseBST::insert(const Expense& expense) {
    return insertHelper(root, expense.getId(), expense.getAmountPaise());
}

bool ExpenseBST::findByAmount(long long amountPaise, int& expenseId) const {
    const BSTNode* curr = root;

    // Standard BST search path
    while (curr) {
        if (curr->getAmountPaise() == amountPaise) {
            // Traversal rule: Return the first matching node encountered along the search path
            expenseId = curr->getExpenseId();
            return true;
        } else if (amountPaise < curr->getAmountPaise()) {
            curr = curr->getLeft();
        } else {
            curr = curr->getRight();
        }
    }

    return false;
}

void ExpenseBST::displayInOrderHelper(const BSTNode* current) const {
    if (!current) return;

    displayInOrderHelper(current->getLeft());

    long long amt = current->getAmountPaise();
    long long rupees = amt / 100;
    long long paise = amt % 100;
    if (paise < 0) paise = -paise;

    std::cout << "  [Amount: Rs." << rupees << "." << (paise < 10 ? "0" : "") << paise
              << " (" << amt << " paise) | Expense ID: " << current->getExpenseId() << "]\n";

    displayInOrderHelper(current->getRight());
}

void ExpenseBST::displayInOrder() const {
    if (!root) {
        std::cout << "  (ExpenseBST is empty)\n";
        return;
    }
    displayInOrderHelper(root);
}

void ExpenseBST::clearHelper(BSTNode* current) {
    if (!current) return;
    clearHelper(current->left);
    clearHelper(current->right);
    delete current;
}

void ExpenseBST::clear() {
    clearHelper(root);
    root = nullptr;
    nodeCount = 0;
}

int ExpenseBST::getCount() const {
    return nodeCount;
}

bool ExpenseBST::isEmpty() const {
    return root == nullptr;
}

const BSTNode* ExpenseBST::getRoot() const {
    return root;
}
