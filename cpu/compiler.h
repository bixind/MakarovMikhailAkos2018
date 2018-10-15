#pragma once

#include "parser.h"

namespace Cpu {
    void Compile(CommandsReader& reader, std::ostream& out) {
        Command command = HLT;
        do {
            reader.NextCommand(&command);
            out.write(reinterpret_cast<const char*>(&command), sizeof(command));
            out.write(reinterpret_cast<const char*>(reader.GetArgs()), sizeof(double) * CommandList[command].param_cnt);
        } while (command != HLT);
    }

    void Decompile(const char* program, std::ostream& out) {
        Command command = HLT;
        BufferCommandsReader reader(program);
        do {
            reader.NextCommand(&command);
            out << StringFromCommand(command, reader.GetArgs()) << "\n";
        } while (command != HLT);
    }
} // Cpu