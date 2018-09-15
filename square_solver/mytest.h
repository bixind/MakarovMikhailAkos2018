#pragma once

#include <vector>
#include <iostream>
#include <stdexcept>

class UnitException: public std::runtime_error {
public:
    UnitException(const std::string& message): runtime_error(message) {}
};

class TestCaseBase {
public:
    virtual void run() = 0;
    virtual std::string getName() const = 0;
};
class TestSuiteBase {
public:
    virtual void runTests() = 0;
};

class TestSuiteHolder {
public:
    void addSuite(TestSuiteBase& suite) {
        suits.push_back(&suite);
    }
    void runSuits() {
        for (auto& suit: suits) {
            suit->runTests();
        }
    }
private:
    std::vector<TestSuiteBase*> suits;
};

TestSuiteHolder& GetTestSuiteHolder() {
    static TestSuiteHolder suiteHolder;
    return suiteHolder;
}


#define RUN_TEST_SUITS()             \
int main() {                         \
    GetTestSuiteHolder().runSuits(); \
    return 0;                        \
}

#define TEST_SUITE(NAME)                                                   \
namespace TestSuite##NAME {                                                \
    class Suite##NAME: public TestSuiteBase {                              \
    public:                                                                \
        void addTest(TestCaseBase& testCase) {                             \
            tests.push_back(&testCase);                                    \
        }                                                                  \
        void runTests() {                                                  \
            size_t passed = 0;                                             \
            size_t failed = 0;                                             \
            for (auto& test: tests) {                                      \
                bool isPassed = true;                                      \
                try {                                                      \
                    test->run();                                           \
                } catch (const UnitException& e) {                         \
                    std::cerr << "Test " << test->getName() << " failed: ";\
                    std::cerr << e.what() << std::endl;                    \
                    isPassed = false;                                      \
                }                                                          \
                if (isPassed)                                              \
                    ++passed;                                              \
                else                                                       \
                    ++failed;                                              \
            }                                                              \
            std::cerr << "Total: " << passed << " passed, ";               \
            std::cerr << failed << " failed." << std::endl;                \
        }                                                                  \
    private:                                                               \
        std::vector<TestCaseBase*> tests;                                  \
    };                                                                     \
    static Suite##NAME SuiteInstance;                                      \
                                                                           \
    struct Suite##NAME##Registrator {                                      \
        Suite##NAME##Registrator() {                                       \
            GetTestSuiteHolder().addSuite(SuiteInstance);                  \
        }                                                                  \
    };                                                                     \
    static Suite##NAME##Registrator Suite##NAME##RegistratorInstance;      \
}                                                                          \
namespace TestSuite##NAME

#define TEST(NAME)                                                       \
class TestCase##NAME: public TestCaseBase {                              \
public:                                                                  \
    void run() override;                                                 \
    std::string getName() const override {                               \
        return std::string(#NAME);                                       \
    }                                                                    \
};                                                                       \
static TestCase##NAME TestCase##NAME##Instance;                          \
                                                                         \
struct TestCase##NAME##Registrator {                                     \
    TestCase##NAME##Registrator() {                                      \
        SuiteInstance.addTest(TestCase##NAME##Instance);                 \
    }                                                                    \
};                                                                       \
static TestCase##NAME##Registrator TestCase##NAME##RegistratorInstance;  \
                                                                         \
void TestCase##NAME::run()

#define UNIT_ASSERT(VALUE)                                    \
do {                                                          \
    if (!(bool) (VALUE)) {                                    \
        throw UnitException("unit assert '"#VALUE"' failed"); \
    }                                                         \
} while(false)
