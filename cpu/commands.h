// COMMAND(NAME, PARAM_CNT, READ, WRITE, CODE)

#define NOARG_COMMAND(NAME, name, CODE) \
COMMAND(NAME, 0, {                      \
    UNUSED(args);                       \
    result =  command_line == name;       \
}, {                                    \
    UNUSED(args);                       \
    result =  name;                       \
}, CODE)

NOARG_COMMAND(HLT, "hlt", {
    UNUSED(args);
})

COMMAND(PUSH, 1, {
    result =  ReadSimpleCommand(command_line, "push", 1, args);
}, {
    result =  WriteSimpleCommand("push", 1, args);
}, {
    stack_.Push(args[0]);
})

NOARG_COMMAND(POP, "pop", {
    UNUSED(args);
    stack_.Pop(nullptr);
})

NOARG_COMMAND(DUP, "dup", {
    UNUSED(args);
    double val = 0;
    stack_.Pop(&val);
    stack_.Push(val);
    stack_.Push(val);
})

NOARG_COMMAND(IN, "in", {
    UNUSED(args);
    double value = 0;
    std::cin >> value;
    stack_.Push(value);
})

NOARG_COMMAND(OUT, "out", {
    UNUSED(args);
    double value = 0;
    stack_.Pop(&value);
    std::cout << value << "\n";
})

NOARG_COMMAND(ADD, "add", {
    UNUSED(args);
    double second = 0;
    double first = 0;
    stack_.Pop(&second);
    stack_.Pop(&first);
    stack_.Push(first + second);
})

NOARG_COMMAND(MUL, "mul", {
    UNUSED(args);
    double second = 0;
    double first = 0;
    stack_.Pop(&second);
    stack_.Pop(&first);
    stack_.Push(first * second);
})

NOARG_COMMAND(DIV, "div", {
    UNUSED(args);
    double second = 0;
    double first = 0;
    stack_.Pop(&second);
    stack_.Pop(&first);
    stack_.Push(first / second);
})

NOARG_COMMAND(SUB, "sub", {
    UNUSED(args);
    double second = 0;
    double first = 0;
    stack_.Pop(&second);
    stack_.Pop(&first);
    stack_.Push(first - second);
})

NOARG_COMMAND(SQRT, "sqrt", {
    UNUSED(args);
    double val = 0;
    stack_.Pop(&val);
    stack_.Push(sqrt(val));
})

NOARG_COMMAND(ABS, "abs", {
    UNUSED(args);
    double val = 0;
    stack_.Pop(&val);
    stack_.Push(fabs(val));
})

//PUSH REG

#define REGISTER(REG, NUM)                \
NOARG_COMMAND(PUSH_##REG, "push " #REG, { \
    UNUSED(args);                         \
    stack_.Push(regs_[REG]);      \
})

#include "registers.h"
#undef REGISTER

//POP REG

#define REGISTER(REG, NUM)                  \
NOARG_COMMAND(POP_##REG, "pop " #REG, {     \
    UNUSED(args);                           \
    stack_.Pop(&(regs_[REG]));      \
})

#include "registers.h"
#undef REGISTER



COMMAND(PUSH_MEM, 1, {
    result =  ReadMemCommand(command_line, "push", "[", "]", args);
}, {
    result =  WriteMemCommand("push", "[", "]", args);
}, {
    stack_.Push(mem_.at(args[0]));
})

COMMAND(POP_MEM, 1, {
    result =  ReadMemCommand(command_line, "pop", "[", "]", args);
}, {
    result =  WriteMemCommand("pop", "[", "]", args);
}, {
    stack_.Pop(&mem_.at(args[0]));
})

//PUSH MEM REG

#define REGISTER(REG, NUM)                              \
NOARG_COMMAND(PUSH_MEM_##REG, "push [" #REG "]", {      \
    UNUSED(args);                                       \
    stack_.Push(mem_.at(regs_[REG]));       \
})

#include "registers.h"
#undef REGISTER

//POP MEM REG

#define REGISTER(REG, NUM)                          \
NOARG_COMMAND(POP_MEM_##REG, "pop [" #REG "]", {    \
    UNUSED(args);                                   \
    stack_.Pop(&(mem_.at(regs_[REG]))); \
})

#include "registers.h"
#undef REGISTER

//PUSH MEM REG OFFSET

#define REGISTER(REG, NUM)                                                \
COMMAND(PUSH_MEM_OFFSET_##REG, 1, {                                       \
    result =  ReadMemCommand(command_line, "push", "[" #REG "+", "]", args); \
}, {                                                                      \
    result =  WriteMemCommand("push", "[" #REG "+", "]", args);              \
}, {                                                                      \
    stack_.Push(mem_.at(regs_[REG] + args[0]));               \
})

#include "registers.h"
#undef REGISTER

//POP MEM REG OFFSET

#define REGISTER(REG, NUM)                                               \
COMMAND(POP_MEM_OFFSET_##REG, 1, {                                       \
    result =  ReadMemCommand(command_line, "pop", "[" #REG "+", "]", args); \
}, {                                                                     \
result =  WriteMemCommand("pop", "[" #REG "+", "]", args);                  \
}, {                                                                     \
    stack_.Pop(&mem_.at(args[0] + regs_[REG]));              \
})

#include "registers.h"
#undef REGISTER

#define JUMP_COMMAND(NAME, name, CODE)                          \
COMMAND(NAME, 1, {                                              \
    UNUSED(args);                                               \
    result =  ReadJumpCommand(command_line, name);                 \
}, {                                                            \
    UNUSED(args);                                               \
    result =  std::string(name) + " " + std::to_string(args[0]);   \
}, CODE)

JUMP_COMMAND(JMP, "jmp", {
    reader_.Jump(args[0]);
})

JUMP_COMMAND(JE, "je", {
    double first;
    double second;
    stack_.Pop(&second);
    stack_.Pop(&first);
    if (first == second) {
        reader_.Jump(args[0]);
    }
})

JUMP_COMMAND(JNE, "jne", {
    double first;
    double second;
    stack_.Pop(&second);
    stack_.Pop(&first);
    if (first != second) {
        reader_.Jump(args[0]);
    }
})

JUMP_COMMAND(JA, "ja", {
    double first;
    double second;
    stack_.Pop(&second);
    stack_.Pop(&first);
    if (first > second) {
        reader_.Jump(args[0]);
    }
})

JUMP_COMMAND(JEXEC, "jexec", {
    regs_[RDX] = reader_.GetNextPosition();
    reader_.Jump(args[0]);
})

NOARG_COMMAND(RET, "ret", {
    UNUSED(args);
    reader_.Jump(regs_[RDX]);
})

#undef JUMP_COMMAND
#undef NOARG_COMMAND
