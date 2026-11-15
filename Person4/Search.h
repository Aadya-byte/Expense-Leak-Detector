#ifndef SEARCH_H
#define SEARCH_H

#include "Expense.h"

/**
 * @file Search.h
 * @brief Search utility class providing linear and binary search algorithms for expenses.
 * 
 * Person 4 Module: Searching, sorting and BST index.
 * Strict no-STL compliance: manually implemented algorithms without <algorithm> or STL containers.
 */
class Search {
public:
    Search();
    ~Search();

    /**
     * @brief Performs linear search for an expense by ID in an array of non-owning Expense pointers.
     * 
     * Works on both unsorted and sorted arrays.
     * Iterates sequentially through the array elements.
     * 
     * @param expenses Array of pointers to Expense objects (may be unsorted).
     * @param count Number of elements in the expenses array.
     * @param id The target expense ID to find.
     * @return Expense* Pointer to the matching Expense object if found; nullptr otherwise.
     * 
     * Time Complexity: O(N) worst and average case, O(1) best case.
     * Auxiliary Space: O(1).
     */
    Expense* linearSearchById(Expense* expenses[], int count, int id) const;

    /**
     * @brief Performs binary search for an expense by ID in a sorted array of Expense pointers.
     * 
     * PRECONDITION:
     * The `sortedExpenses` array MUST be sorted in ascending order by expense ID (`id`)
     * prior to calling this function (e.g. sorted using Sort::sortById()). If the array
     * is unsorted, the search behavior is undefined and may fail to locate existing records.
     * 
     * @param sortedExpenses Array of Expense pointers sorted by ID in ascending order.
     * @param count Number of elements in the array.
     * @param id The target expense ID to find.
     * @return Expense* Pointer to the matching Expense object if found; nullptr otherwise.
     * 
     * Time Complexity: O(log N) worst and average case, O(1) best case.
     * Auxiliary Space: O(1).
     */
    Expense* binarySearchById(Expense* sortedExpenses[], int count, int id) const;
};

#endif // SEARCH_H
