#include <cmath>
#include <cassert>

constexpr double EPSILON = 1e-7;

/// Checks whether double value is zero
bool isZero(double a) {
    return fabs(a) < EPSILON;
}

/** Solves linear equations.
 *
 * @param [in]  polynom polynomial coefficients in degree increasing order
 * @param [out] answer  polinomial roots
 * @return number of roots
 */
int solveLinearPolynom(const double *polynom, double *answer) {
    // assert that input is correct
    assert(std::isfinite(polynom[0]));
    assert(std::isfinite(polynom[1]));
    if (isZero(polynom[1])) { // constant polynomial
        return isZero(polynom[0]) ? -1 : 0;
    }
    answer[0] = -polynom[0] / polynom[1];
    return 1;
}

/** Solves square equations.
 *
 * @param [in]  polynom polynomial coefficients in degree increasing order
 * @param [out] answer  polinomial roots in increasing order
 * @return number of roots
 */
int solveSquarePolynom(const double *polynom, double *answer) {
    // assert that input is correct
    assert(std::isfinite(polynom[0]));
    assert(std::isfinite(polynom[1]));
    assert(std::isfinite(polynom[2]));
    if (isZero(polynom[2]))
        return solveLinearPolynom(polynom, answer);
    double a = polynom[2], b = polynom[1], c = polynom[0];
    double discriminant = b * b - 4 * c * a;
    if (isZero(discriminant)) { // one root
        answer[0] = -b / (2 * a);
        return 1;
    }
    if (discriminant < 0) { // no roots
        return 0;
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
    return 2;
}
