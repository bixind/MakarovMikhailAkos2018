#include <gtest/gtest.h>
#include "main.h"

void assertLess(const char* a, const char* b) {
    ASSERT_TRUE(lexicographicalComparator(a, b));
    ASSERT_FALSE(lexicographicalComparator(b, a));
}

TEST(LexicographicalComparatorCase, AlnumTest) {
    {
        const char *a = "aaab";
        const char *b = "aaac";
        assertLess(a, b);
    }
    {
        const char *a = "aaa";
        const char *b = "aaac";
        assertLess(a, b);
    }
    {
        const char *a = "";
        const char *b = "aaac";
        assertLess(a, b);
    }
    {
        const char *a = "";
        const char *b = "";
        ASSERT_FALSE(lexicographicalComparator(a, b));
        ASSERT_FALSE(lexicographicalComparator(b, a));
    }
    {
        const char *a = "asDs";
        const char *b = "asDs";
        ASSERT_FALSE(lexicographicalComparator(a, b));
        ASSERT_FALSE(lexicographicalComparator(b, a));
    }
    {
        const char *a = "uiiifeijefji23erferf";
        const char *b = "uiiifeijefji45";
        assertLess(a, b);
    }
}

TEST(LexicographicalComparatorCase, PunctuationTest) {
    {
        const char *a = ".,.,.a,.a,.a,.b.,";
        const char *b = ".,,a.,.a,.a,.c";
        assertLess(a, b);
    }
    {
        const char *a = ",.a.,a,.a,.";
        const char *b = ".,;,.a  a ac";
        assertLess(a, b);
    }
    {
        const char *a = "';;' ;' ";
        const char *b = " ;'a' '.,a.,ac";
        assertLess(a, b);
    }
    {
        const char *a = ";' ,. ";
        const char *b = "'; '; ";
        ASSERT_FALSE(lexicographicalComparator(a, b));
        ASSERT_FALSE(lexicographicalComparator(b, a));
    }
    {
        const char *a = "   ;'  a,. s,. D s .";
        const char *b = " ;;a' 's  .D .s";
        ASSERT_FALSE(lexicographicalComparator(a, b));
        ASSERT_FALSE(lexicographicalComparator(b, a));
    }
    {
        const char *a = "ui;' ; ii ;' f;' eijefji23erferf";
        const char *b = ";' 'ui,'; 'i;' ifeijefji45";
        assertLess(a, b);
    }
}

void assertReverseLess(const char* a, const char* b) {
    ASSERT_TRUE(reverseLexicographicalComparator(a, b));
    ASSERT_FALSE(reverseLexicographicalComparator(b, a));
}

TEST(ReverseLexicographicalComparatorCase, AlnumTest) {
    {
        const char *a = "aaab";
        const char *b = "aaac";
        assertReverseLess(a, b);
    }
    {
        const char *a = "aaac";
        const char *b = "aaz";
        assertReverseLess(a, b);
    }
    {
        const char *a = "";
        const char *b = "aaac";
        assertReverseLess(a, b);
    }
    {
        const char *a = "";
        const char *b = "";
        ASSERT_FALSE(reverseLexicographicalComparator(a, b));
        ASSERT_FALSE(reverseLexicographicalComparator(b, a));
    }
    {
        const char *a = "asDs";
        const char *b = "asDs";
        ASSERT_FALSE(reverseLexicographicalComparator(a, b));
        ASSERT_FALSE(reverseLexicographicalComparator(b, a));
    }
    {
        const char *a = "zzzuiiifei23444";
        const char *b = "aer45444";
        assertReverseLess(a, b);
    }
}

TEST(ReverseLexicographicalComparatorCase, PunctuationTest) {
    {
        const char *a = "a;;aa;;;b;";
        const char *b = ",.aa,.a,.,c,.";
        assertReverseLess(a, b);
    }
    {
        const char *a = "aaac.,.";
        const char *b = "a,.a,.z;";
        assertReverseLess(a, b);
    }
    {
        const char *a = ",.,.,.";
        const char *b = "a,.a,.a,.c";
        assertReverseLess(a, b);
    }
    {
        const char *a = ",.,.";
        const char *b = ";';;';';";
        ASSERT_FALSE(reverseLexicographicalComparator(a, b));
        ASSERT_FALSE(reverseLexicographicalComparator(b, a));
    }
    {
        const char *a = ",.a,.sD,.s";
        const char *b = "a,.s;;D,.s";
        ASSERT_FALSE(reverseLexicographicalComparator(a, b));
        ASSERT_FALSE(reverseLexicographicalComparator(b, a));
    }
    {
        const char *a = "z.,zzuii.,ifei.,234,.44";
        const char *b = ",.ae.,;r.,454.,44";
        assertReverseLess(a, b);
    }
}

std::unique_ptr<char[]> prepareString(const char* string) {
    size_t len = strlen(string);
    std::unique_ptr<char[]> buf(new char[len + 1]);
    memcpy(buf.get(), string, len + 1);
    return buf;
}

TEST(ExtractLinesCase, SimpleTest) {
    {
        auto buf = prepareString("sample string");
        auto lines = extractLines(buf.get());
        std::vector<const char*> answer = {buf.get()};
        ASSERT_EQ(lines, answer);
    }
    {
        auto buf = prepareString("12345\n6789\nsample");
        auto lines = extractLines(buf.get());
        std::vector<const char*> answer = {buf.get(), buf.get() + 6, buf.get() + 11};
        ASSERT_EQ(lines, answer);
    }
    {
        auto buf = prepareString("\n\n\nsample\n");
        auto lines = extractLines(buf.get());
        std::vector<const char*> answer = {buf.get(), buf.get() + 1, buf.get() + 2, buf.get() + 3, buf.get() + 7};
        ASSERT_EQ(lines, answer);
    }
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}