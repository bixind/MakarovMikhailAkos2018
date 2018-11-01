#include <err.h>
#include <fstream>
#include "compiler.h"

int main(int argc, const char** argv) {
    if (argc != 3) {
        errx(1, "Exactly 2 arguments expected: input and output file");
    }
    std::ifstream in(argv[1]);
    std::ofstream out(argv[2]);
    out << Cpu::Compile(in);
}