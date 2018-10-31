#include <gtest/gtest.h>
#include <fstream>
#include "parser.h"
#include "compiler.h"

std::string RunProgram(Cpu::CommandsReader& reader, const std::string& input) {
    // replacing stdout and stdin
    std::stringstream input_buffer(input);
    std::stringstream output_buffer;
    std::streambuf* old_input = std::cin.rdbuf(input_buffer.rdbuf());
    std::streambuf* old_output = std::cout.rdbuf(output_buffer.rdbuf());
    std::cin.tie(nullptr);

    Cpu::Cpu cpu(reader);
    cpu.Run();

    // put everything back
    std::cin.rdbuf(old_input);
    std::cout.rdbuf(old_output);
    return output_buffer.str();
}

void TestTextProgram(const std::string& program, const std::string& input, const std::string& output) {
    std::stringstream program_stream(program);
    auto bytecode = Cpu::Compile(program_stream);
    Cpu::BufferCommandsReader reader(bytecode.data());

    auto result = RunProgram(reader, input);

    ASSERT_EQ(result, output);
}

void TestBinaryProgram(const char* program, const std::string& input, const std::string& output) {
    Cpu::BufferCommandsReader reader(program);

    auto result = RunProgram(reader, input);

    ASSERT_EQ(result, output);
}

std::string CompileFromFile(const std::string& filename) {
    std::ifstream in(filename);
    return Cpu::Compile(in);
}

TEST(CommandTest, AllCommands) {
    TestTextProgram(
        "push 10\n"
        "push 20\n"
        "add\n"
        "out\n"
        "hlt\n"
        ,
        ""
        ,
        "30\n"
    );
    TestTextProgram(
        "push 10\n"
        "push 20\n"
        "sub\n"
        "out\n"
        "hlt\n"
        ,
        ""
        ,
        "-10\n"
    );
    TestTextProgram(
        "push 10\n"
        "push 20\n"
        "mul\n"
        "out\n"
        "hlt\n"
        ,
        ""
        ,
        "200\n"
    );
    TestTextProgram(
        "push 40\n"
        "push 20\n"
        "div\n"
        "out\n"
        "hlt\n"
        ,
        ""
        ,
        "2\n"
    );
    TestTextProgram(
        "push 10\n"
        "push 20\n"
        "pop\n"
        "out\n"
        "hlt\n"
        ,
        ""
        ,
        "10\n"
    );
    TestTextProgram(
        "in\n"
        "in\n"
        "add\n"
        "out\n"
        "hlt\n"
        ,
        "10\n"
        "31\n"
        ,
        "41\n"
    );
}

TEST(CommandTest, RegistersAndMem) {
    TestTextProgram(
        "in\n"
        "in\n"
        "in\n"
        "pop RAX\n"
        "pop RBX\n"
        "pop RCX\n"
        "push RAX\n"
        "push RBX\n"
        "push RCX\n"
        "out\n"
        "out\n"
        "out\n"
        "hlt\n"
        ,
        "1\n"
        "2\n"
        "3\n"
        ,
        "1\n"
        "2\n"
        "3\n"
    );
    TestTextProgram(
        "push 5\n"
        "pop [1]\n"
        "push [1]\n"
        "out\n"
        "hlt\n"
        ,
        ""
        ,
        "5\n"
    );
    TestTextProgram(
        "push 5\n"
        "pop RAX\n"
        "push 4\n"
        "pop [RAX]\n"
        "push 6\n"
        "push [RAX]\n"
        "out\n"
        "hlt\n"
        ,
        ""
        ,
        "4\n"
    );
    TestTextProgram(
        "push 5\n"
        "pop RAX\n"
        "push 4\n"
        "pop [RAX+12]\n"
        "push 6\n"
        "push [RAX+12]\n"
        "out\n"
        "hlt\n"
        ,
        ""
        ,
        "4\n"
    );
}

TEST(CommandTest, Jump) {
    TestTextProgram(
        "push 5\n"
        "jmp there\n"
        "push 4\n"
        ":there\n"
        "out\n"
        "hlt\n"
        ,
        ""
        ,
        "5\n"
    );
    TestTextProgram(
        "push 3\n"
        "pop RAX\n"
        ":start\n"
        "push RAX\n"
        "push 1\n"
        "sub\n"
        "pop RAX\n"
        "push RAX\n"
        "out\n"
        "push RAX\n"
        "push 0\n"
        "ja start\n"
        "hlt\n"
        ,
        ""
        ,
        "2\n"
        "1\n"
        "0\n"
    );
    TestTextProgram(
        "push 4\n"
        "jexec mul2\n"
        "out\n"
        "hlt\n"
        ":mul2\n"
        "push 2\n"
        "mul\n"
        "ret\n"
        ,
        ""
        ,
        "8\n"
    );
}

TEST(Compiler, CompileDecompileCompile) {
    std::string program =
        "push 10\n"
        "push 20\n"
        "push 30\n"
        "add\n"
        "mul\n"
        "out\n"
        "hlt\n"
    ;
    std::stringstream in(program);
    std::stringstream out_decomp;
    auto out1 = Cpu::Compile(in);
    Cpu::Decompile(out1.data(), out_decomp);
    std::stringstream in2(out_decomp.str());
    auto out2 = Cpu::Compile(in2);
    ASSERT_EQ(out1, out2);
}

TEST(BigPrograms, Fib) {
    auto fib_program = CompileFromFile("../cpu/test_programs/fib.txt");
    TestBinaryProgram(fib_program.data(), "0\n", "1\n");
    TestBinaryProgram(fib_program.data(), "1\n", "1\n");
    TestBinaryProgram(fib_program.data(), "2\n", "2\n");
    TestBinaryProgram(fib_program.data(), "3\n", "3\n");
    TestBinaryProgram(fib_program.data(), "4\n", "5\n");
    TestBinaryProgram(fib_program.data(), "5\n", "8\n");
    TestBinaryProgram(fib_program.data(), "6\n", "13\n");
}

TEST(BigPrograms, SquareSolver) {
    auto solver_program = CompileFromFile("../cpu/test_programs/square_solver.txt");

    TestBinaryProgram(solver_program.data(), "1\n-4\n3\n", "2\n1\n3\n");
    TestBinaryProgram(solver_program.data(), "12\n-1\n-1\n", "2\n-0.25\n0.333333\n");
    TestBinaryProgram(solver_program.data(), "1\n6\n9\n", "1\n-3\n");
    TestBinaryProgram(solver_program.data(), "1\n6\n10\n", "0\n");

    TestBinaryProgram(solver_program.data(), "0\n2\n10\n", "1\n-5\n");

    TestBinaryProgram(solver_program.data(), "0\n0\n5\n", "0\n");
    TestBinaryProgram(solver_program.data(), "0\n0\n0\n", "-1\n");
}

TEST(Runner, BinFormat) {
    std::string program =
        "push 10\n"
        "push 20\n"
        "push 30\n"
        "add\n"
        "mul\n"
        "out\n"
        "hlt\n"
    ;
    std::stringstream in(program);
    auto out = Cpu::Compile(in);
    TestBinaryProgram(
        out.data(),
        "",
        "500\n"
    );
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}