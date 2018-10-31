#pragma once

#include "parser.h"

namespace Cpu {
    std::string Compile(std::istream& in) {
        Command command = HLT;
        std::string program;
        std::map<std::string, size_t> labels;
        std::vector<std::pair<size_t, std::string> > places;
        double args[MAX_ARGS_COUNT];
        while (in) {
            std::string command_line;
            while (in.good() && command_line.empty()) {
                std::getline(in, command_line);
                if (!command_line.empty() && command_line[0] == ':') {
                    auto result = labels.emplace(command_line.substr(1), program.size());
                    assert(result.second);
                    command_line.clear();
                }
            }
            if (in.eof()) {
                break;
            }
            CommandFromString(command_line, &command, args);

            if (command_line[0] == 'j') {
                auto label = GetJumpLabel(command_line);
                places.emplace_back(program.size() + 1, label);
                args[0] = 0;
            }
            program.append(reinterpret_cast<const char*>(&command), sizeof(command));
            program.append(reinterpret_cast<const char*>(args), sizeof(double) * CommandList[command].param_cnt);
        }
        for (const auto& place : places) {
            auto it = labels.find(place.second);
            assert(it != labels.end());
            *reinterpret_cast<double*>(&program[place.first]) = it->second;
        }
        return program;
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