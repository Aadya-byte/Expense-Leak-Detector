#ifndef SORT_H
#define SORT_H

#include "Expense.h"

/**
 * @file Sort.h
 * @brief Sorting utility class providing manual sorting algorithms for expenses.
 * 
 * Person 4 Module: Searching, sorting and BST index.
 * Strict no-STL compliance: manually implemented algorithms without <algorithm> or std::sort.
 * 
 * Important safety contract:
 * Sorting rearranges non-owning pointers in the supplied array. It must not modify
 * the master linked list in ExpenseManager or change the Expense records themselves.
 */
class Sort {
public:
    Sort();
    ~Sort();

    /**
     * @brief Sorts an array of Expense pointers in ascending order by expense ID.
     * 
     * Uses manually implemented Quicksort with middle-pivot selection to prevent
     * O(N^2) degradation on already-sorted or reverse-sorted data.
     * 
     * @param expenses Array of non-owning Expense pointers.
     * @param count Number of elements in the array.
     * 
     * Time Complexity: O(N log N) average and best case, O(N^2) worst case.
     * Auxiliary Space: O(log N) recursion stack space.
     */
    void sortById(Expense* expenses[], int count) const;

    /**
     * @brief Sorts an array of Expense pointers in ascending order by amount in paise.
     * 
     * Uses expense ID as a deterministic secondary tie-breaker so duplicate amounts
     * are ordered consistently.
     * 
     * @param expenses Array of non-owning Expense pointers.
     * @param count Number of elements in the array.
     * 
     * Time Complexity: O(N log N) average and best case, O(N^2) worst case.
     * Auxiliary Space: O(log N) recursion stack space.
     */
    void sortByAmount(Expense* expenses[], int count) const;

    /**
     * @brief Sorts an array of Expense pointers in ascending chronological order by date.
     * 
     * Parses dates in "DD-MM-YYYY" format and orders from earliest to latest.
     * Uses expense ID as a secondary tie-breaker for identical dates.
     * 
     * @param expenses Array of non-owning Expense pointers.
     * @param count Number of elements in the array.
     * 
     * Time Complexity: O(N log N) average and best case, O(N^2) worst case.
     * Auxiliary Space: O(log N) recursion stack space.
     */
    void sortByDate(Expense* expenses[], int count) const;

private:
    // Manual comparison helpers
    static int compareById(const Expense* a, const Expense* b);
    static int compareByAmount(const Expense* a, const Expense* b);
    static int compareByDate(const Expense* a, const Expense* b);
    static long long parseDateToKey(const char* date);

    // Manual quicksort engine
    typedef int (*Comparator)(const Expense*, const Expense*);
    void quickSort(Expense* arr[], int low, int high, Comparator comp) const;
};

#endif // SORT_H
