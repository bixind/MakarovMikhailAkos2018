#include <gtest/gtest.h>
#include "parser.h"
#include "compiler.h"

void TestTextProgram(const std::string& program, const std::string& input, const std::string& output) {
    // replacing stdout and stdin
    std::stringstream input_buffer(input);
    std::stringstream output_buffer;
    std::streambuf* old_input = std::cin.rdbuf(input_buffer.rdbuf());
    std::streambuf* old_output = std::cout.rdbuf(output_buffer.rdbuf());
    std::cin.tie(nullptr);

    std::stringstream program_stream(program);
    Cpu::StreamCommandsReader reader(program_stream);

    Cpu::Cpu cpu(reader);
    cpu.Run();

    // put everything back
    std::cin.rdbuf(old_input);
    std::cout.rdbuf(old_output);

    ASSERT_EQ(output_buffer.str(), output);
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
    std::stringstream out_binary1, out_binary2;
    std::stringstream out_decomp;
    Cpu::StreamCommandsReader reader1(in);
    Cpu::Compile(reader1, out_binary1);
    Cpu::Decompile(out_binary1.str().data(), out_decomp);
    Cpu::StreamCommandsReader reader2(out_decomp);
    Cpu::Compile(reader2, out_binary2);
    ASSERT_EQ(out_binary1.str(), out_binary2.str());
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}