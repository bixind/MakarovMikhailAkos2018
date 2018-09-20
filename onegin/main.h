#pragma once

#include <memory>
#include <sys/stat.h>
#include <vector>
#include <err.h>

/** Allocates buffer and reads whole file in it.
 *  Aborts the program on any error.
 *
 * @param [in] filename name of file
 * @return char buffer with file
 */
std::unique_ptr<char[]> readFile(const char* filename) {
    struct stat statbuf;
    int res = stat(filename, &statbuf);
    if (res < 0) {
        err(1, "Error calling stat");
    }
    if (!S_ISREG(statbuf.st_mode)) {
        errx(1, "Regular file expected");
    }
    std::unique_ptr<char[]> fileBuffer(new char[statbuf.st_size + 1]);
    FILE* file = fopen(filename, "r");
    if (!file) {
        err(1, "Failed to open file");
    }
    size_t fileSize = fread(fileBuffer.get(), 1, statbuf.st_size, file);
    fclose(file);
    if (ferror(file)) {
        err(1, "Error when reading/closing file");
    }
    fileBuffer[fileSize] = '\0';
    return fileBuffer;
}

/** Prints c-strings from strings, separated by newlines,
 *  to file filename in write mode. Empty lines are not printed.
 *  Aborts the program on any error.
 *
 * @param [in] filename name of file
 * @param [in] strings string to print in file
 */
void printToFile(const char* filename, const std::vector<const char*>& strings) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        err(1, "Failed to write file");
    }
    int code = 0;
    for (const char* string: strings) {
        if (*string != '\0') {
            code = fprintf(file, "%s\n", string);
            if (code < 0)
                break;
        }
    }
    fclose(file);
    if (code < 0) {
        err(1, "Failed to write to file");
    }
}

/** Splits the string 'file' inplace by newline character,
 *  by replacing them with '\0', then returns a vector of pointers
 *  on the starts of those strings
 *
 * @param [in] file input string
 * @return vector of c-string pointers
 */
std::vector<const char*> extractLines(char* file) {
    size_t cnt = 0;
    size_t length = 0;
    for (size_t i = 0; file[i] != '\0'; ++i) {
        if (file[i] == '\n') {
            file[i] = '\0';
            ++cnt;
        }
        ++length;
    }
    ++cnt;
    std::vector<const char*> result(cnt, nullptr);
    result[0] = file;
    size_t cur = 1;
    for (size_t i = 1; i <= length; ++i) {
        if (file[i - 1] == '\0') {
            result[cur++] = file + i;
        }
    }
    return result;
}

/** Compares two c-strings in lexicographical order,
 *  skipping over non alphabetic and non numeric characters.
 *
 * @param a first c-string
 * @param b second c-string
 * @return is a less than b
 */
bool lexicographicalComparator(const char* a, const char* b) {
    while (true) {
        if (*a != '\0' && !isalnum(*a)) {
            ++a;
            continue;
        }
        if (*b != '\0' && !isalnum(*b)) {
            ++b;
            continue;
        }
        if (*b == '\0')
            return false;
        if (*a == '\0')
            return true;
        if (*a != *b)
            return *a < *b;
        ++a;
        ++b;
    }
}

/** Compares the reversal of two c-strings in lexicographical order,
 *  skipping over non alphabetic and non numeric characters.
 *
 * @param a first c-string
 * @param b second c-string
 * @return is reversed a less than reversed b
 */
bool reverseLexicographicalComparator(const char* aStart, const char* bStart) {
    const char* a = aStart;
    const char* b = bStart;
    while (*a != '\0')
        ++a;
    while (*b != '\0')
        ++b;
    while (true) {
        if (a != aStart && !isalnum(*(a - 1))) {
            --a;
            continue;
        }
        if (b != bStart && !isalnum(*(b - 1))) {
            --b;
            continue;
        }
        if (b == bStart)
            return false;
        if (a == aStart)
            return true;
        if (*(a - 1) != *(b - 1))
            return *(a - 1) < *(b - 1);
        --a;
        --b;
    }
}