// COMMAND(NAME, PARAM_CNT, READ, WRITE, CODE)

#define NOARG_COMMAND(NAME, name, CODE) \
COMMAND(NAME, 0, {                      \
    UNUSED(args);                       \
    return command_line == name;       \
}, {                                    \
    UNUSED(args);                       \
    return name;                       \
}, CODE)

NOARG_COMMAND(HLT, "hlt", {
    UNUSED(cpu);
    UNUSED(args);
})

COMMAND(PUSH, 1, {
    return ReadSimpleCommand(command_line, "push", 1, args);
}, {
    return WriteSimpleCommand("push", 1, args);
}, {
    cpu.stack_.Push(args[0]);
})

NOARG_COMMAND(POP, "pop", {
    UNUSED(args);
    cpu.stack_.Pop(nullptr);
})

NOARG_COMMAND(DUP, "dup", {
    UNUSED(args);
    double val = 0;
    cpu.stack_.Pop(&val);
    cpu.stack_.Push(val);
    cpu.stack_.Push(val);
})

NOARG_COMMAND(IN, "in", {
    UNUSED(args);
    double value = 0;
    std::cin >> value;
    cpu.stack_.Push(value);
})

NOARG_COMMAND(OUT, "out", {
    UNUSED(args);
    double value = 0;
    cpu.stack_.Pop(&value);
    std::cout << value << "\n";
})

NOARG_COMMAND(ADD, "add", {
    UNUSED(args);
    double second = 0;
    double first = 0;
    cpu.stack_.Pop(&second);
    cpu.stack_.Pop(&first);
    cpu.stack_.Push(first + second);
})

NOARG_COMMAND(MUL, "mul", {
    UNUSED(args);
    double second = 0;
    double first = 0;
    cpu.stack_.Pop(&second);
    cpu.stack_.Pop(&first);
    cpu.stack_.Push(first * second);
})

NOARG_COMMAND(DIV, "div", {
    UNUSED(args);
    double second = 0;
    double first = 0;
    cpu.stack_.Pop(&second);
    cpu.stack_.Pop(&first);
    cpu.stack_.Push(first / second);
})

NOARG_COMMAND(SUB, "sub", {
    UNUSED(args);
    double second = 0;
    double first = 0;
    cpu.stack_.Pop(&second);
    cpu.stack_.Pop(&first);
    cpu.stack_.Push(first - second);
})

NOARG_COMMAND(SQRT, "sqrt", {
    UNUSED(args);
    double val = 0;
    cpu.stack_.Pop(&val);
    cpu.stack_.Push(sqrt(val));
})

NOARG_COMMAND(ABS, "abs", {
    UNUSED(args);
    double val = 0;
    cpu.stack_.Pop(&val);
    cpu.stack_.Push(fabs(val));
})

//PUSH REG

#define REGISTER(REG, NUM)                \
NOARG_COMMAND(PUSH_##REG, "push " #REG, { \
    UNUSED(args);                         \
    cpu.stack_.Push(cpu.regs_[REG]);      \
})

#include "registers.h"
#undef REGISTER

//POP REG

#define REGISTER(REG, NUM)                  \
NOARG_COMMAND(POP_##REG, "pop " #REG, {     \
    UNUSED(args);                           \
    cpu.stack_.Pop(&(cpu.regs_[REG]));      \
})

#include "registers.h"
#undef REGISTER



COMMAND(PUSH_MEM, 1, {
    return ReadMemCommand(command_line, "push", "[", "]", args);
}, {
    return WriteMemCommand("push", "[", "]", args);
}, {
    cpu.stack_.Push(cpu.mem_.at(args[0]));
})

COMMAND(POP_MEM, 1, {
    return ReadMemCommand(command_line, "pop", "[", "]", args);
}, {
    return WriteMemCommand("pop", "[", "]", args);
}, {
    cpu.stack_.Pop(&cpu.mem_.at(args[0]));
})

//PUSH MEM REG

#define REGISTER(REG, NUM)                              \
NOARG_COMMAND(PUSH_MEM_##REG, "push [" #REG "]", {      \
    UNUSED(args);                                       \
    cpu.stack_.Push(cpu.mem_.at(cpu.regs_[REG]));       \
})

#include "registers.h"
#undef REGISTER

//POP MEM REG

#define REGISTER(REG, NUM)                          \
NOARG_COMMAND(POP_MEM_##REG, "pop [" #REG "]", {    \
    UNUSED(args);                                   \
    cpu.stack_.Pop(&(cpu.mem_.at(cpu.regs_[REG]))); \
})

#include "registers.h"
#undef REGISTER

//PUSH MEM REG OFFSET

#define REGISTER(REG, NUM)                                                \
COMMAND(PUSH_MEM_OFFSET_##REG, 1, {                                       \
    return ReadMemCommand(command_line, "push", "[" #REG "+", "]", args); \
}, {                                                                      \
    return WriteMemCommand("push", "[" #REG "+", "]", args);              \
}, {                                                                      \
    cpu.stack_.Push(cpu.mem_.at(cpu.regs_[REG] + args[0]));               \
})

#include "registers.h"
#undef REGISTER

//POP MEM REG OFFSET

#define REGISTER(REG, NUM)                                               \
COMMAND(POP_MEM_OFFSET_##REG, 1, {                                       \
    return ReadMemCommand(command_line, "pop", "[" #REG "+", "]", args); \
}, {                                                                     \
return WriteMemCommand("pop", "[" #REG "+", "]", args);                  \
}, {                                                                     \
    cpu.stack_.Pop(&cpu.mem_.at(args[0] + cpu.regs_[REG]));              \
})

#include "registers.h"
#undef REGISTER

#define JUMP_COMMAND(NAME, name, CODE)                          \
COMMAND(NAME, 1, {                                              \
    UNUSED(args);                                               \
    return ReadJumpCommand(command_line, name);                 \
}, {                                                            \
    UNUSED(args);                                               \
    return std::string(name) + " " + std::to_string(args[0]);   \
}, CODE)

JUMP_COMMAND(JMP, "jmp", {
    cpu.reader_.Jump(args[0]);
})

JUMP_COMMAND(JE, "je", {
    double first;
    double second;
    cpu.stack_.Pop(&second);
    cpu.stack_.Pop(&first);
    if (first == second) {
        cpu.reader_.Jump(args[0]);
    }
})

JUMP_COMMAND(JNE, "jne", {
    double first;
    double second;
    cpu.stack_.Pop(&second);
    cpu.stack_.Pop(&first);
    if (first != second) {
        cpu.reader_.Jump(args[0]);
    }
})

JUMP_COMMAND(JA, "ja", {
    double first;
    double second;
    cpu.stack_.Pop(&second);
    cpu.stack_.Pop(&first);
    if (first > second) {
        cpu.reader_.Jump(args[0]);
    }
})

JUMP_COMMAND(JEXEC, "jexec", {
    cpu.regs_[RDX] = cpu.reader_.GetNextPosition();
    cpu.reader_.Jump(args[0]);
})

NOARG_COMMAND(RET, "ret", {
    UNUSED(args);
    cpu.reader_.Jump(cpu.regs_[RDX]);
})

#undef JUMP_COMMAND
#undef NOARG_COMMAND
