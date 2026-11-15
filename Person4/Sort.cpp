#include "Sort.h"

Sort::Sort() {}

Sort::~Sort() {}

long long Sort::parseDateToKey(const char* date) {
    if (!date) return 0;

    // Check minimum length for "DD-MM-YYYY"
    int len = 0;
    while (date[len] != '\0' && len < 10) {
        len++;
    }
    if (len < 10) return 0;

    // Parse DD, MM, YYYY
    int day = (date[0] - '0') * 10 + (date[1] - '0');
    int month = (date[3] - '0') * 10 + (date[4] - '0');
    int year = (date[6] - '0') * 1000 + (date[7] - '0') * 100 +
               (date[8] - '0') * 10 + (date[9] - '0');

    if (day < 1 || day > 31 || month < 1 || month > 12 || year < 0) {
        return 0; // Fallback for invalid numeric conversion
    }

    return (long long)year * 10000 + month * 100 + day;
}

int Sort::compareById(const Expense* a, const Expense* b) {
    if (a == b) return 0;
    if (!a) return 1;  // nulls pushed to the end
    if (!b) return -1;

    if (a->getId() < b->getId()) return -1;
    if (a->getId() > b->getId()) return 1;
    return 0;
}

int Sort::compareByAmount(const Expense* a, const Expense* b) {
    if (a == b) return 0;
    if (!a) return 1;
    if (!b) return -1;

    if (a->getAmountPaise() < b->getAmountPaise()) return -1;
    if (a->getAmountPaise() > b->getAmountPaise()) return 1;

    // Secondary tie-breaker: ID ascending
    if (a->getId() < b->getId()) return -1;
    if (a->getId() > b->getId()) return 1;
    return 0;
}

int Sort::compareByDate(const Expense* a, const Expense* b) {
    if (a == b) return 0;
    if (!a) return 1;
    if (!b) return -1;

    long long keyA = parseDateToKey(a->getDate());
    long long keyB = parseDateToKey(b->getDate());

    if (keyA < keyB) return -1;
    if (keyA > keyB) return 1;

    // Secondary tie-breaker: ID ascending
    if (a->getId() < b->getId()) return -1;
    if (a->getId() > b->getId()) return 1;
    return 0;
}

void Sort::quickSort(Expense* arr[], int low, int high, Comparator comp) const {
    if (low >= high) return;

    // Middle pivot selection mitigates worst-case O(N^2) on sorted/reverse-sorted inputs
    Expense* pivot = arr[low + (high - low) / 2];
    int i = low;
    int j = high;

    while (i <= j) {
        while (comp(arr[i], pivot) < 0) {
            i++;
        }
        while (comp(arr[j], pivot) > 0) {
            j--;
        }
        if (i <= j) {
            Expense* temp = arr[i];
            arr[i] = arr[j];
            arr[j] = temp;
            i++;
            j--;
        }
    }

    if (low < j) {
        quickSort(arr, low, j, comp);
    }
    if (i < high) {
        quickSort(arr, i, high, comp);
    }
}

void Sort::sortById(Expense* expenses[], int count) const {
    if (!expenses || count <= 1) return;
    quickSort(expenses, 0, count - 1, compareById);
}

void Sort::sortByAmount(Expense* expenses[], int count) const {
    if (!expenses || count <= 1) return;
    quickSort(expenses, 0, count - 1, compareByAmount);
}

void Sort::sortByDate(Expense* expenses[], int count) const {
    if (!expenses || count <= 1) return;
    quickSort(expenses, 0, count - 1, compareByDate);
}
