#include "mytest.h"
#include "main.h"

TEST_SUITE(SquarePolynomial) {
    TEST(TwoRoots) {
        {
            double polynom[] = {3, -4, 1};
            double answer[2];
            UNIT_ASSERT(solveSquarePolynom(polynom, answer) == 2);
            UNIT_ASSERT(isZero(answer[0] - 1));
            UNIT_ASSERT(isZero(answer[1] - 3));
        }
        {
            double polynom[] = {-1, -1, 12};
            double answer[2];
            UNIT_ASSERT(solveSquarePolynom(polynom, answer) == 2);
            UNIT_ASSERT(isZero(answer[0] + 1.0/4));
            UNIT_ASSERT(isZero(answer[1] - 1.0/3));
        }
    }

    TEST(OneRoot) {
        double polynom[] = {9, 6, 1};
        double answer[2];
        UNIT_ASSERT(solveSquarePolynom(polynom, answer) == 1);
        UNIT_ASSERT(isZero(answer[0] + 3));
    }

    TEST(NoRoots) {
        double polynom[] = {10, 6, 1};
        double answer[2];
        UNIT_ASSERT(solveSquarePolynom(polynom, answer) == 0);
    }
}

TEST_SUITE(LinearPolynomial) {
    TEST(OneRoot) {
        RootsCount(*solvers[])(const double*, double*) = {solveSquarePolynom, solveLinearPolynom};
        {
            for (size_t i = 0; i < 2; ++i) {
                double polynom[] = {5, 1, 0};
                double answer[2];
                UNIT_ASSERT(solvers[i](polynom, answer) == 1);
                UNIT_ASSERT(isZero(answer[0] + 5));
            }
        }
        {
            for (size_t i = 0; i < 2; ++i) {
                double polynom[] = {-7, 6, 0};
                double answer[2];
                UNIT_ASSERT(solvers[i](polynom, answer) == 1);
                UNIT_ASSERT(isZero(answer[0] - 7.0/6));
            }
        }
    }

    TEST(NoRoots) {
        RootsCount(*solvers[])(const double*, double*) = {solveSquarePolynom, solveLinearPolynom};
        for (size_t i = 0; i < 2; ++i) {
            double polynom[] = {5, 0, 0};
            double answer[2];
            UNIT_ASSERT(solvers[i](polynom, answer) == 0);
        }
    }

    TEST(InfRoots) {
        RootsCount(*solvers[])(const double*, double*) = {solveSquarePolynom, solveLinearPolynom};
        for (size_t i = 0; i < 2; ++i) {
            double polynom[] = {0, 0, 0};
            double answer[2];
            UNIT_ASSERT(solvers[i](polynom, answer) == -1);
        }
    }
}

RUN_TEST_SUITS()
