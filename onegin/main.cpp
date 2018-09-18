#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>
#include <memory>
#include <err.h>
#include <vector>
#include <algorithm>

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
    std::vector<const char*> result(cnt, nullptr);
    result[0] = file;
    size_t cur = 1;
    for (size_t i = 1; i < length; ++i) {
        if (file[i - 1] == '\0') {
            result[cur++] = file + i;
        }
    }
    return result;
}

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

int main() {
    printf("# File lines alphabetical sorter\n");
    printf("Enter the file name\n");
    char *buf = NULL;
    size_t bufferLength = 0;
    ssize_t res = getline(&buf, &bufferLength, stdin);
    std::unique_ptr<char[]> filename(buf);
    if (res <= 0) {
        err(1, "Failed to read file name");
    }
    if (res == 0) {
        errx(1, "Provided filename is empty");
    }
    if (filename[res - 1] == '\n') {
        filename[res - 1] = '\0';
        --res;
    }
    printf("%s\n", filename.get());
    std::unique_ptr<char[]> fileBuffer = readFile(filename.get());
    std::vector<const char*> lines = extractLines(fileBuffer.get());
    std::sort(lines.begin(), lines.end(), lexicographicalComparator);
    for (auto w: lines) {
        printf("%s\n", w);
    }
    return 0;
}