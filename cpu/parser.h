#pragma once

#include "../stack/stack.h"
#include "../onegin/main.h"
#include <string>
#include <cassert>
#include <iostream>
#include <cstring>
#include "string"
#include <sstream>
#include <map>
#include <cmath>

#define UNUSED(x) (void)(x)

namespace Cpu {
    using CpuStack = Stack<double>;
    const size_t MAX_COMMAND_COUNT = 256;
    const size_t MAX_ARGS_COUNT = 2;
    const size_t MAX_STRING_LENGTH = 1000;
    const size_t REGISTER_COUNT = 0
#define REGISTER(NAME, NUM) + 1
#include "registers.h"
#undef REGISTER
;
    class Cpu;

    enum Register {
#define REGISTER(NAME, NUM) NAME = NUM,
#include "registers.h"
#undef REGISTER
        NOREG = 99
    };

    Register RegisterFromString(const std::string& name) {
#define REGISTER(NAME, NUM)  \
        if (#NAME == name)   \
            return NAME;
#include "registers.h"
#undef REGISTER
        return NOREG;
    }

std::string StringFromRegister(Register reg) {
    switch (reg) {
#define REGISTER(NAME, NUM)   \
        case NAME:        \
            return #NAME;
#include "registers.h"
#undef REGISTER
        case NOREG:
            assert(false);
    }
}

    bool ReadSimpleCommand(const std::string& command_line, const std::string& name, int arg_num, double* args) {
        std::stringstream command_stream(command_line);
        std::string command_name;
        command_stream >> command_name;
        if (command_name != name) {
            return false;
        }
        for (int i = 0; i < arg_num && command_stream.good(); ++i) {
            command_stream >> args[i];
        }
        return command_stream && command_stream.eof();
    }

    std::string WriteSimpleCommand(const std::string& name, int arg_num, const double* args) {
        std::stringstream command_stream;
        command_stream << name << ' ';
        for (int i = 0; i < arg_num && command_stream.good(); ++i) {
            command_stream << args[i];
        }
        return command_stream.str();
    }

    bool ReadMemCommand(const std::string& command_line, const std::string& name,
                        const std::string& left_bracket, const std::string& right_bracket, double* args) {
        std::stringstream command_stream(command_line);
        std::string command_name;
        command_stream >> command_name;
        if (command_name != name) {
            return false;
        }
        for (size_t i = 0; i < left_bracket.size(); ++i) {
            char c;
            command_stream >> c;
            if (c != left_bracket[i]) {
                return false;
            }
        }
        command_stream >> args[0];
        for (size_t i = 0; i < right_bracket.size(); ++i) {
            char c;
            command_stream >> c;
            if (c != right_bracket[i]) {
                return false;
            }
        }
        command_stream.peek();
        return command_stream && command_stream.eof();
    }

    std::string WriteMemCommand(const std::string& name,
                                const std::string& left_bracket, const std::string& right_bracket, const double* args) {
        std::stringstream command_stream;
        command_stream << name << ' ' << left_bracket;
        command_stream << args[0] << right_bracket;
        return command_stream.str();
    }

    bool ReadJumpCommand(const std::string& command_line, const std::string& name) {
        std::stringstream command_stream(command_line);
        std::string command_name;
        command_stream >> command_name;
        if (command_name != name) {
            return false;
        }
        std::string label;
        command_stream >> label;
        if (label.empty()) {
            return false;
        }
        return command_stream && command_stream.eof();
    }
    std::string GetJumpLabel(const std::string& command_line) {
        std::stringstream command_stream(command_line);
        std::string label;
        command_stream >> label;
        assert(!label.empty());
        label.clear();
        command_stream >> label;
        assert(!label.empty());
        return label;
    }

    enum Command : uint8_t {
#define COMMAND(NAME, PARAM_CNT, READ, WRITE, CODE) \
NAME,

#include "commands.h"

#undef COMMAND
    };

    struct CommandInfo {
        Command command{Command(0)};
        size_t param_cnt{0};

        void (*command_implementation)(Cpu&, const double *) { nullptr };
        bool (*from_string)(const std::string&, double*) {nullptr};
        std::string (*to_string)(const double*) {nullptr};
    };

    static CommandInfo CommandList[MAX_COMMAND_COUNT] = {};

    void CommandFromString(const std::string& command_line, Command *command, double *args) {
        for (size_t i = 0; i < MAX_COMMAND_COUNT; ++i) {
            if (CommandList[i].from_string &&
                CommandList[i].from_string(command_line, args)) {
                *command = Command(i);
                return;
            }
        }
        // No match found
        assert(false);
    }

    std::string StringFromCommand(Command command, const double *args) {
        assert(CommandList[command].to_string);
        return CommandList[command].to_string(args);
    }

    class CommandsReader {
    public:
        virtual ~CommandsReader() = default;

        virtual void NextCommand(Command *command) = 0;

        virtual const double* GetArgs() const = 0;

        virtual void Jump(size_t pos) = 0;

        virtual size_t GetNextPosition() const = 0;

        virtual std::pair<const char*, size_t> GetCompiled() const = 0;
    };


    class BufferCommandsReader : public CommandsReader {
    public:
        explicit BufferCommandsReader(const char* program)
            : position_(0), last_args_count_(0), buffer_(program) {
        }

        void NextCommand(Command *command) override {
            position_ += last_args_count_ * sizeof(double);
            *command = Command(buffer_[position_]);
            last_args_count_ = CommandList[*command].param_cnt;
            ++position_;
        }

        const double* GetArgs() const override {
            return reinterpret_cast<const double*>(buffer_ + position_);
        }

        void Jump(size_t pos) override {
            position_ = pos;
            last_args_count_ = 0;
        }

        size_t GetNextPosition() const override {
            return position_  + sizeof(double) * last_args_count_;
        }

        std::pair<const char*, size_t> GetCompiled() const override {
            return {buffer_, position_};
        }

    protected:
        size_t position_;
        size_t last_args_count_;
        const char* buffer_;
    };

    class BinaryFileCommandsReader : public BufferCommandsReader {
    public:
        explicit BinaryFileCommandsReader(const char* filename)
            : BufferCommandsReader(nullptr), buffer_holder_(readFile(filename)) {
            buffer_ = buffer_holder_.get();
        }

    private:
        std::unique_ptr<char[]> buffer_holder_;
    };

    class Memory {
    public:
        double& at(size_t pos) {
            if (pos >= buffer.size()) {
                buffer.resize(pos + 1);
            }
            return buffer[pos];
        }
    private:
        std::vector<double> buffer;
    };

    class Cpu {
#define COMMAND(NAME, PARAM_CNT, READ, WRITE, CODE)     \
    friend void Command##NAME##Implementation(Cpu& cpu, \
                                   const double* args);

#include "commands.h"

#undef COMMAND

    public:
        explicit Cpu(CommandsReader &reader) : reader_(reader) {
        }

        void Run() {
            Command command = HLT;
            do {
                reader_.NextCommand(&command);
                CommandList[command].command_implementation(*this, reader_.GetArgs());
            } while (command != HLT);
        }

    private:
        CommandsReader& reader_;
        CpuStack stack_;
        double regs_[REGISTER_COUNT];
        Memory mem_;
    };

#define COMMAND(NAME, PARAM_CNT, READ, WRITE, CODE)                      \
void Command##NAME##Implementation(Cpu& cpu,                             \
                                   const double* args)                   \
                                   CODE;                                 \
bool Command##NAME##FromString(const std::string& command_line,          \
                               double* args)                             \
                               READ;                                     \
std::string Command##NAME##ToString(const double* args)                  \
                               WRITE;                                    \
                                                                         \
struct CommandRegistrator##NAME {                                        \
    CommandRegistrator##NAME() {                                         \
        auto& info = CommandList[NAME];                                  \
        assert(info.command_implementation == nullptr);                  \
        info.command = NAME;                                             \
        info.param_cnt = PARAM_CNT;                                      \
        info.command_implementation = Command##NAME##Implementation;     \
        info.from_string = Command##NAME##FromString;                    \
        info.to_string = Command##NAME##ToString;                        \
    }                                                                    \
};                                                                       \
                                                                         \
static CommandRegistrator##NAME NAME##Registrator;

#include "commands.h"

#undef COMMAND
} // namespace Cpu