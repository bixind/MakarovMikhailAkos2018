/**
 *  @file
 *
 *  Header with linear and square solvers' implementations.
 */

#include <cmath>
#include <cassert>

constexpr double EPSILON = 1e-7;

/// Checks whether double value is zero
bool isZero(double a) {
    return fabs(a) < EPSILON;
}

enum RootsCount {
    NO_ROOTS = 0,
    ONE_ROOT = 1,
    TWO_ROOTS = 2,
    INFINITE_ROOTS = -1
};

/**
 * Solves linear equations.
 *
 * @param [in]  polynom polynomial coefficients in degree increasing order
 * @param [out] answer  polinomial roots
 * @return number of roots
 */
RootsCount solveLinearPolynom(const double* polynom, double* answer) {
    // assert that input is correct
    assert(polynom);
    assert(answer);
    assert(std::isfinite(polynom[0]));
    assert(std::isfinite(polynom[1]));
    if (isZero(polynom[1])) { // constant polynomial
        return isZero(polynom[0]) ? INFINITE_ROOTS : NO_ROOTS;
    }
    answer[0] = -polynom[0] / polynom[1];
    return ONE_ROOT;
}

/**
 * Solves square equations.
 *
 * @param [in]  polynom polynomial coefficients in degree increasing order
 * @param [out] answer  polinomial roots in increasing order
 * @return number of roots
 */
RootsCount solveSquarePolynom(const double* polynom, double* answer) {
    // assert that input is correct
    assert(polynom);
    assert(answer);
    assert(std::isfinite(polynom[0]));
    assert(std::isfinite(polynom[1]));
    assert(std::isfinite(polynom[2]));
    if (isZero(polynom[2]))
        return solveLinearPolynom(polynom, answer);
    double a = polynom[2], b = polynom[1], c = polynom[0];
    double discriminant = b * b - 4 * c * a;
    if (isZero(discriminant)) { // one root
        answer[0] = -b / (2 * a);
        return ONE_ROOT;
    }
    if (discriminant < 0) { // no roots
        return NO_ROOTS;
    }
    // 2 roots
    answer[0] = (-b + sqrt(discriminant)) / (2 * a);
    answer[1] = (-b - sqrt(discriminant)) / (2 * a);
    // ensure increasing order
    if (answer[0] > answer[1]) {
        c = answer[1];
        answer[1] = answer[0];
        answer[0] = c;
    }
    return TWO_ROOTS;
}
