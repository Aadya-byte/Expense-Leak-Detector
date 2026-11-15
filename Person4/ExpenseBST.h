#ifndef EXPENSEBST_H
#define EXPENSEBST_H

#include "Expense.h"
#include "BSTNode.h"

/**
 * @file ExpenseBST.h
 * @brief Binary Search Tree index for expenses, indexed by monetary amount in paise.
 * 
 * Person 4 Module: Searching, sorting and BST index.
 * Strict no-STL compliance: manually implemented binary search tree.
 * 
 * DESIGN PRINCIPLES:
 * 1. The BST stores ONLY indexing values (expenseId, amountPaise).
 *    It does NOT duplicate or own Expense objects.
 * 2. Composite Search Key: `(amountPaise, expenseId)`.
 *    Using expenseId as a secondary tie-breaker enables full support for
 *    duplicate expense amounts in the tree.
 * 3. Traversal Rule for findByAmount():
 *    The method searches down the BST. The first node encountered whose
 *    `amountPaise` matches the query amount is returned as the matching record.
 */
class ExpenseBST {
private:
    BSTNode* root;
    int nodeCount;

    // Recursive helper functions
    bool insertHelper(BSTNode*& current, int id, long long amount);
    void clearHelper(BSTNode* current);
    void displayInOrderHelper(const BSTNode* current) const;
    BSTNode* copyTreeHelper(const BSTNode* otherNode);

public:
    // Constructors & Destructor
    ExpenseBST();
    ExpenseBST(const ExpenseBST& other);
    ExpenseBST& operator=(const ExpenseBST& other);
    ~ExpenseBST();

    /**
     * @brief Inserts an expense's indexing values into the BST.
     * 
     * Key is `(amountPaise, expenseId)`. If an exact duplicate with the same
     * amount and the same ID already exists, insertion is rejected (returns false).
     * 
     * @param expense Reference to the Expense record to index.
     * @return true if insertion succeeded; false if duplicate or allocation failure.
     * 
     * Time Complexity: O(h) where h is tree height (O(log N) average, O(N) worst).
     * Space Complexity: O(1) auxiliary per node.
     */
    bool insert(const Expense& expense);

    /**
     * @brief Searches for an expense ID matching a target amount in paise.
     * 
     * TRAVERSAL RULE:
     * Navigates the BST comparing `amountPaise`. Returns the first node encountered
     * along the standard BST search path whose amount matches the query amount.
     * If multiple records share the same amount, this traversal rule deterministically
     * selects the ancestor / first matching record along the descent path.
     * 
     * @param amountPaise Target amount in integer paise.
     * @param expenseId [out] Reference populated with the found expense ID if true is returned.
     * @return true if a matching record was found; false otherwise.
     * 
     * Time Complexity: O(h) where h is tree height (O(log N) average, O(N) worst).
     * Space Complexity: O(1) iterative lookup.
     */
    bool findByAmount(long long amountPaise, int& expenseId) const;

    /**
     * @brief Prints all indexed records in ascending order of amount (and ID for ties).
     * 
     * Performs in-order traversal (Left, Current, Right).
     * Time Complexity: O(N).
     * Space Complexity: O(h) recursion stack space.
     */
    void displayInOrder() const;

    /**
     * @brief Deletes all nodes and frees dynamically allocated memory.
     * 
     * Performs post-order deallocation. Leaves the tree in an empty state (root = nullptr).
     */
    void clear();

    // Utility getters
    int getCount() const;
    bool isEmpty() const;
    const BSTNode* getRoot() const;
};

#endif // EXPENSEBST_H
