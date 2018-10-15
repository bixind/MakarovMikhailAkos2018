#pragma once

#include "../stack/stack.h"
#include "../onegin/main.h"
#include <string>
#include <cassert>
#include <iostream>
#include <cstring>

#define UNUSED(x) (void)(x)

namespace Cpu {
    using CpuStack = Stack<double>;
    const size_t MAX_COMMAND_COUNT = 256;
    const size_t MAX_ARGS_COUNT = 2;
    const size_t MAX_STRING_LENGTH = 1000;

    enum Command : uint8_t {
        HLT = 0,
        PUSH,
        POP,
        IN,
        OUT,
        ADD,
        MUL,
        DIV,
        SUB
    };

    struct CommandInfo {
        Command command{Command(0)};
        const char *pattern;
        size_t param_cnt{0};

        void (*command_implementation)(CpuStack &, const double *) { nullptr };
    };

    static CommandInfo CommandList[MAX_COMMAND_COUNT] = {};

#define REGISTER_COMMAND(NAME, PATTERN, PARAM_CNT)                       \
void Command##NAME##Implementation(CpuStack& stack,                      \
                                   const double* args);                  \
                                                                         \
struct CommandRegistrator##NAME {                                        \
    CommandRegistrator##NAME() {                                         \
        auto& info = CommandList[NAME];                                  \
        assert(info.command_implementation == nullptr);                  \
        info.command = NAME;                                             \
        info.pattern = PATTERN "%n";                                     \
        info.param_cnt = PARAM_CNT;                                      \
        info.command_implementation = Command##NAME##Implementation;     \
    }                                                                    \
};                                                                       \
                                                                         \
static CommandRegistrator##NAME NAME##Registrator;                       \
                                                                         \
void Command##NAME##Implementation(CpuStack& stack,                      \
                                   const double* args)

    namespace {
        REGISTER_COMMAND(HLT, "hlt", 0) {
            UNUSED(stack);
            UNUSED(args);
        }

        REGISTER_COMMAND(PUSH, "push %lf", 1) {
            stack.Push(args[0]);
        }

        REGISTER_COMMAND(POP, "pop", 0) {
            UNUSED(args);
            stack.Pop(nullptr);
        }

        REGISTER_COMMAND(IN, "in", 0) {
            UNUSED(args);
            double value = 0;
            std::cin >> value;
            stack.Push(value);
        }

        REGISTER_COMMAND(OUT, "out", 0) {
            UNUSED(args);
            double value = 0;
            stack.Pop(&value);
            std::cout << value << "\n";
        }

        REGISTER_COMMAND(ADD, "add", 0) {
            UNUSED(args);
            double second = 0;
            double first = 0;
            stack.Pop(&second);
            stack.Pop(&first);
            stack.Push(first + second);
        }

        REGISTER_COMMAND(MUL, "mul", 0) {
            UNUSED(args);
            double second = 0;
            double first = 0;
            stack.Pop(&second);
            stack.Pop(&first);
            stack.Push(first * second);
        }

        REGISTER_COMMAND(DIV, "div", 0) {
            UNUSED(args);
            double second = 0;
            double first = 0;
            stack.Pop(&second);
            stack.Pop(&first);
            stack.Push(first / second);
        }

        REGISTER_COMMAND(SUB, "sub", 0) {
            UNUSED(args);
            double second = 0;
            double first = 0;
            stack.Pop(&second);
            stack.Pop(&first);
            stack.Push(first - second);
        }
    }

    void CommandFromString(const char *command_line, Command *command, double *args) {
        for (size_t i = 0; i < MAX_COMMAND_COUNT; ++i) {
            int read = 0;
            switch (CommandList[i].param_cnt) {
                case 0:
                    sscanf(command_line, CommandList[i].pattern, &read);
                    break;
                case 1:
                    sscanf(command_line, CommandList[i].pattern, &(args[0]), &read);
                    break;
                case 2:
                    sscanf(command_line, CommandList[i].pattern, &(args[0]), &(args[1]), &read);
                    break;
                default:
                    // Unexpected args number
                    assert(false);
            }
            if (read == static_cast<int>(strlen(command_line))) {
                *command = Command(i);
                return;
            }
        }
        // No match found
        assert(false);
    }

    const char* StringFromCommand(Command command, const double *args) {
        assert(CommandList[command].command_implementation);
        static char buffer[MAX_STRING_LENGTH] = "";
        auto& info = CommandList[command];
        int printed = 0;
        switch (info.param_cnt) {
            case 0:
                sprintf(buffer, info.pattern, &printed);
                break;
            case 1:
                sprintf(buffer, info.pattern, args[0], &printed);
                break;
            case 2:
                sprintf(buffer, info.pattern, args[0], args[1], &printed);
                break;
            default:
                // Unexpected args number
                assert(false);
        }
        if (printed > 0) {
            return buffer;
        }
        // failed to print
        assert(false);
        return nullptr;
    }

    class CommandsReader {
    public:
        virtual ~CommandsReader() = default;

        virtual void NextCommand(Command *command) = 0;

        virtual const double* GetArgs() const = 0;
    };

    class StreamCommandsReader : public CommandsReader {
    public:
        explicit StreamCommandsReader(std::istream &stream) : in_(stream) {
        }

        void NextCommand(Command *command) override {
            std::string command_line;
            while (command_line.empty()) {
                assert(in_.good());
                std::getline(in_, command_line);
            }
            CommandFromString(command_line.data(), command, command_args_);
        }

        const double* GetArgs() const override {
            return command_args_;
        }

    private:
        std::istream &in_;
        double command_args_[MAX_ARGS_COUNT];

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

    class Cpu {
    public:
        explicit Cpu(CommandsReader &reader) : reader_(reader) {
        }

        void Run() {
            Command command = HLT;
            do {
                reader_.NextCommand(&command);
                CommandList[command].command_implementation(stack_, reader_.GetArgs());
            } while (command != HLT);
        }

    private:
        CommandsReader& reader_;
        CpuStack stack_;
    };
} // namespace Cpu