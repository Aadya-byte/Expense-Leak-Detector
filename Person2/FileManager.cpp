/*
 * ============================================================================
 * FILE MANAGER MODULE IMPLEMENTATION
 * File: FileManager.cpp
 * Owned by: Person 2 (Persistence & Export Module)
 * ============================================================================
 */

#include "FileManager.h"
#include "../Person1/ExpenseManager.h"
#include "../Person1/ExpenseNode.h"
#include "RecordParser.h"
#include "../Person1/Expense.h"

#include <fstream>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <climits>

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#include <direct.h>
#include <windows.h>
#define MKDIR(dir) _mkdir(dir)
#else
#include <sys/stat.h>
#define MKDIR(dir) mkdir(dir, 0755)
#endif

FileManager::FileManager(const char* filePath) {
    m_lastError[0] = '\0';
    setFilePath(filePath);
}

FileManager::~FileManager() {
}

const char* FileManager::getLastError() const {
    return m_lastError;
}

const char* FileManager::getFilePath() const {
    return m_filePath;
}

void FileManager::setFilePath(const char* filePath) {
    if (filePath != NULL && *filePath != '\0') {
        std::strncpy(m_filePath, filePath, sizeof(m_filePath) - 1);
        m_filePath[sizeof(m_filePath) - 1] = '\0';
    } else {
        std::strncpy(m_filePath, "expense.txt", sizeof(m_filePath) - 1);
        m_filePath[sizeof(m_filePath) - 1] = '\0';
    }
}

void FileManager::setLastError(const char* error) {
    if (error != NULL) {
        std::strncpy(m_lastError, error, sizeof(m_lastError) - 1);
        m_lastError[sizeof(m_lastError) - 1] = '\0';
    } else {
        m_lastError[0] = '\0';
    }
}

void FileManager::clearLastError() {
    m_lastError[0] = '\0';
}

bool FileManager::ensureParentDirectoryExists(const char* filePath) {
    if (filePath == NULL) return false;

    // Search for last directory separator
    const char* lastSlash = NULL;
    for (const char* p = filePath; *p != '\0'; ++p) {
        if (*p == '/' || *p == '\\') {
            lastSlash = p;
        }
    }

    if (lastSlash == NULL) {
        // Current directory; no parent directory creation needed
        return true;
    }

    // Extract directory path
    char dirPath[260];
    int len = (int)(lastSlash - filePath);
    if (len >= (int)sizeof(dirPath)) return false;

    std::strncpy(dirPath, filePath, len);
    dirPath[len] = '\0';

    // Recursively or iteratively create directories
    for (int i = 0; i < len; ++i) {
        if (dirPath[i] == '/' || dirPath[i] == '\\') {
            char temp = dirPath[i];
            dirPath[i] = '\0';
            if (dirPath[0] != '\0') {
                MKDIR(dirPath);
            }
            dirPath[i] = temp;
        }
    }
    MKDIR(dirPath);
    return true;
}

bool FileManager::loadExpenses(ExpenseManager& manager) {
    clearLastError();

    // 1. Open the file for reading
    errno = 0;
    std::ifstream inFile(m_filePath, std::ios::in);
    if (!inFile.is_open()) {
        if (errno == ENOENT) {
            clearLastError();
            return true;
        }
        char errBuf[512];
        std::snprintf(errBuf, sizeof(errBuf), "Failed to open file for reading: '%s'", m_filePath);
        setLastError(errBuf);
        return false;
    }

    // Check for empty file
    inFile.seekg(0, std::ios::end);
    std::streampos fileSize = inFile.tellg();
    if (fileSize == 0) {
        // Safe handling of empty file: clear manager and succeed with 0 records
        inFile.close();
        manager.clearExpenses();
        clearLastError();
        return true;
    }
    inFile.seekg(0, std::ios::beg);

    // Dynamic array for duplicate checking during validation (Zero STL)
    int capacity = 32;
    int seenCount = 0;
    int* seenIds = new int[capacity];

    RecordParser parser;
    char lineBuffer[1024];
    int lineNumber = 0;
    bool hasValidRecords = false;

    // ------------------------------------------------------------------------
    // PASS 1: Strict Validation Pass
    // Check every line, format, length, value and uniqueness before touching manager.
    // ------------------------------------------------------------------------
    while (inFile.getline(lineBuffer, sizeof(lineBuffer))) {
        lineNumber++;

        // Trim trailing carriage return if present (Windows/Unix line ending compatibility)
        int len = (int)std::strlen(lineBuffer);
        if (len > 0 && lineBuffer[len - 1] == '\r') {
            lineBuffer[len - 1] = '\0';
            len--;
        }

        // Allow blank/whitespace lines without failing
        bool isBlank = true;
        for (int i = 0; i < len; ++i) {
            if (lineBuffer[i] != ' ' && lineBuffer[i] != '\t') {
                isBlank = false;
                break;
            }
        }
        if (isBlank) {
            continue;
        }

        Expense tempExpense;
        if (!parser.parseRecord(lineBuffer, tempExpense)) {
            char errBuf[512];
            std::snprintf(errBuf, sizeof(errBuf), "Line %d: Malformed expense record", lineNumber);
            setLastError(errBuf);
            delete[] seenIds;
            inFile.close();
            // Crucial: In-memory manager dataset is NOT touched
            return false;
        }

        int expId = tempExpense.getId();

        // Check for duplicate ID within file
        for (int i = 0; i < seenCount; ++i) {
            if (seenIds[i] == expId) {
                char errBuf[512];
                std::snprintf(errBuf, sizeof(errBuf), "Line %d: Duplicate expense ID %d", lineNumber, expId);
                setLastError(errBuf);
                delete[] seenIds;
                inFile.close();
                // Crucial: In-memory manager dataset is NOT touched
                return false;
            }
        }

        // Expand dynamic array if needed
        if (seenCount >= capacity) {
            int newCapacity = capacity * 2;
            int* newSeen = new int[newCapacity];
            for (int i = 0; i < seenCount; ++i) {
                newSeen[i] = seenIds[i];
            }
            delete[] seenIds;
            seenIds = newSeen;
            capacity = newCapacity;
        }
        seenIds[seenCount++] = expId;
        hasValidRecords = true;
    }
    if (inFile.fail() && !inFile.eof()) {
        setLastError("A record exceeds the maximum supported line length");
        delete[] seenIds;
        inFile.close();
        return false;
    }

    // Clean up temporary seen IDs array
    delete[] seenIds;
    seenIds = NULL;

    // If file only contained whitespace/blank lines
    if (!hasValidRecords) {
        inFile.close();
        manager.clearExpenses();
        clearLastError();
        return true;
    }

    // ------------------------------------------------------------------------
    // PASS 2: Safe Commit Pass
    // Entire file is verified. Now we can safely clear the manager and insert.
    // ------------------------------------------------------------------------
    manager.clearExpenses();

    inFile.clear();
    inFile.seekg(0, std::ios::beg);
    lineNumber = 0;

    while (inFile.getline(lineBuffer, sizeof(lineBuffer))) {
        lineNumber++;
        int len = (int)std::strlen(lineBuffer);
        if (len > 0 && lineBuffer[len - 1] == '\r') {
            lineBuffer[len - 1] = '\0';
            len--;
        }

        bool isBlank = true;
        for (int i = 0; i < len; ++i) {
            if (lineBuffer[i] != ' ' && lineBuffer[i] != '\t') {
                isBlank = false;
                break;
            }
        }
        if (isBlank) continue;

        Expense tempExpense;
        parser.parseRecord(lineBuffer, tempExpense);

        // Insert into manager preserving saved ID
        if (!manager.addLoadedExpense(tempExpense)) {
            char errBuf[512];
            std::snprintf(errBuf, sizeof(errBuf), "Line %d: Failed to insert loaded expense ID %d into manager",
                          lineNumber, tempExpense.getId());
            setLastError(errBuf);
            inFile.close();
            return false;
        }
    }

    inFile.close();
    clearLastError();
    return true;
}

bool FileManager::saveExpenses(const ExpenseManager& manager) {
    clearLastError();

    // Ensure parent directory exists
    if (!ensureParentDirectoryExists(m_filePath)) {
        char errBuf[512];
        std::snprintf(errBuf, sizeof(errBuf), "Failed to create directory for file: '%s'", m_filePath);
        setLastError(errBuf);
        return false;
    }

    // Prepare temporary snapshot file path
    char tempPath[280];
    std::snprintf(tempPath, sizeof(tempPath), "%s.tmp", m_filePath);

    // Open temporary snapshot file for writing
    std::ofstream outFile(tempPath, std::ios::out | std::ios::trunc);
    if (!outFile.is_open()) {
        char errBuf[512];
        std::snprintf(errBuf, sizeof(errBuf), "Failed to open temporary file for writing: '%s'", tempPath);
        setLastError(errBuf);
        return false;
    }

    // Write all records directly from the manager singly linked list
    const ExpenseNode* curr = manager.getHead();
    while (curr != NULL) {
        const Expense& exp = curr->getExpense();
        outFile << exp.getId() << '|'
                << exp.getDate() << '|'
                << exp.getAmountPaise() << '|'
                << exp.getMerchant() << '|'
                << exp.getCategory() << '|'
                << exp.getExpenseType() << '|'
                << exp.getFrequency() << '\n';

        if (!outFile.good()) {
            char errBuf[512];
            std::snprintf(errBuf, sizeof(errBuf), "Write error occurred while writing snapshot to '%s'", tempPath);
            setLastError(errBuf);
            outFile.close();
            std::remove(tempPath);
            return false;
        }
        curr = curr->getNext();
    }

    outFile.flush();
    if (outFile.fail()) {
        char errBuf[512];
        std::snprintf(errBuf, sizeof(errBuf), "Failed to flush snapshot file '%s'", tempPath);
        setLastError(errBuf);
        outFile.close();
        std::remove(tempPath);
        return false;
    }
    outFile.close();

    // Replace the target atomically without deleting the last good snapshot first.
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
    bool replaced = MoveFileExA(tempPath, m_filePath,
                                MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
#else
    bool replaced = std::rename(tempPath, m_filePath) == 0;
#endif
    if (!replaced) {
        char errBuf[512];
        std::snprintf(errBuf, sizeof(errBuf), "Failed to replace target file '%s' with temporary snapshot '%s'",
                      m_filePath, tempPath);
        setLastError(errBuf);
        std::remove(tempPath);
        return false;
    }

    clearLastError();
    return true;
}
