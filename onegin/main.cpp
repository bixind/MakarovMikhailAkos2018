#include <sys/types.h>
#include <cstdio>
#include <memory>
#include <err.h>
#include <vector>
#include <algorithm>
#include <cstring>
#include "main.h"

int main() {
    printf("# File alphabetical sorter\n");
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
    }
    printf("%s\n", filename.get());
    std::unique_ptr<char[]> fileBuffer = readFile(filename.get());
    std::vector<const char*> lines = extractLines(fileBuffer.get());
    {
        std::string fileCopy;
        const char* suffix = "_copy";
        fileCopy.reserve(strlen(filename.get()) + strlen(suffix));
        fileCopy.append(filename.get()).append(suffix);
        printToFile(fileCopy.data(), lines);
    }
    {
        std::sort(lines.begin(), lines.end(), lexicographicalComparator);
        std::string sortedFile;
        const char* suffix = "_sorted";
        sortedFile.reserve(strlen(filename.get()) + strlen(suffix));
        sortedFile.append(filename.get()).append(suffix);
        printToFile(sortedFile.data(), lines);
    }
    {
        std::sort(lines.begin(), lines.end(), reverseLexicographicalComparator);
        std::string sortedFile;
        const char* suffix = "_reverse_sorted";
        sortedFile.reserve(strlen(filename.get()) + strlen(suffix));
        sortedFile.append(filename.get()).append(suffix);
        printToFile(sortedFile.data(), lines);
    }
    return 0;
}