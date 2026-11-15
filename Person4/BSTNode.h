#ifndef BSTNODE_H
#define BSTNODE_H

/**
 * @file BSTNode.h
 * @brief Node for the binary search tree index.
 * 
 * Stores only index values: expenseId and amountPaise.
 * Does NOT own an Expense object.
 */
class BSTNode {
private:
    int expenseId;
    long long amountPaise;
    BSTNode* left;
    BSTNode* right;

public:
    // Constructors & Destructor
    BSTNode();
    BSTNode(int id, long long amount);
    ~BSTNode();

    // Minimum public getters
    int getExpenseId() const;
    long long getAmountPaise() const;
    BSTNode* getLeft() const;
    BSTNode* getRight() const;

    // Setters
    void setLeft(BSTNode* newLeft);
    void setRight(BSTNode* newRight);
    void setExpenseId(int id);
    void setAmountPaise(long long amount);

    // Grant ExpenseBST access to internal pointers for efficient tree operations
    friend class ExpenseBST;
};

#endif // BSTNODE_H
