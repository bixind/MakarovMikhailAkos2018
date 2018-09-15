#include "main.h"
#include <cstdio>

int main() {
    printf("# Square equation solver\n");
    double polynom[3];
    printf("# Enter a, b, c \n");
    int err = scanf("%lf %lf %lf", polynom + 2, polynom + 1, polynom);
    if (err == EOF) {
        printf("Unexpected EOF\n");
        return 1;
    }
    if (err != 3) {
        printf("Failed to read coefficents\n");
        return 1;
    }
    if (!std::isfinite(polynom[0]) || !std::isfinite(polynom[1]) || !std::isfinite(polynom[2])) {
        printf("Finite arguments expected\n");
        return 1;
    }
    double answer[2];
    int rootCount = solveSquarePolynom(polynom, answer);
    switch (rootCount) {
        case -1:
            printf("Any number\n");
            break;
        case 0:
            printf("No roots\n");
            break;
        case 1:
            printf("x = %lf\n", answer[0]);
            break;
        case 2:
            printf("x1 = %lf, x2 = %lf\n", answer[0], answer[1]);
            break;
        default:
            printf("Error: unexpected number of roots: %d\n", rootCount);
            return 1;
    }
    return 0;
}
