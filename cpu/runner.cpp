#include <err.h>
#include "parser.h"

int main(int argc, const char** argv) {
    if (argc != 2) {
        errx(1, "Exactly 1 argument expected: script file");
    }
    Cpu::BinaryFileCommandsReader reader(argv[1]);
    Cpu::Cpu cpu(reader);
    cpu.Run();
}