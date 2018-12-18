#include <gtest/gtest.h>
#include "trees.h"
#include <random>

TEST(parsing, simple) {
    std::string text = "(((3.000000) + (sin(x))) * (5.000000))";
    auto tree = TreeFromString(text);
    ASSERT_EQ(text, tree.PrintTree());
}

TEST(differentiating, simple) {
    std::string text = "(((y) + (sin(x))) * (x))";
    auto tree = TreeFromString(text).Differentiate("x");
    ASSERT_EQ("((((0.000000) + ((cos(x)) * (1.000000))) * (x)) + (((y) + (sin(x))) * (1.000000)))", tree.PrintTree());
    tree.Simplify();
    ASSERT_EQ("(((cos(x)) * (x)) + ((y) + (sin(x))))", tree.PrintTree());
}

TEST(differentiating, hard) {
    std::string text = "(((x) - ((2) * (x))) + ((x) / (1)))";
    auto tree = TreeFromString(text);
    tree = tree.Differentiate("x");
    tree.Simplify();
    ASSERT_EQ("(0.000000)", tree.PrintTree());
}

TEST(differentiating, tan) {
    std::string text = "((tan(x)) * (tan(x)))";
    auto tree = TreeFromString(text);
    tree = tree.Differentiate("x");
    tree.Simplify();
    ASSERT_EQ("((((1.000000) / ((cos(x)) * (cos(x)))) * (tan(x))) + ((tan(x)) * ((1.000000) / ((cos(x)) * (cos(x))))))", tree.PrintTree());
}

TEST(simplify, simple) {
    std::string text = "(minus(cos(0)))";
    auto tree = TreeFromString(text);
    tree.Simplify();
    ASSERT_EQ("(-1.000000)", tree.PrintTree());
}

TEST(simplify, hard) {
    std::string text = "((minus(cos(0))) * (sin((1) / (3))))";
    auto tree = TreeFromString(text);
    tree.Simplify();
    ASSERT_EQ("(-0.327195)", tree.PrintTree());
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}