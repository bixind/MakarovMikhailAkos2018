#include "stack.h"
#include <iostream>

int main() {
    Stack<int> stack;
    size_t n;
    std::cin >> n;
    for (size_t i = 0; i < n; ++i) {
        int a;
        std::cin >> a;
        stack.Push(a);
    }
//    (*(int**)(((int*) &stack) + 6))[0] = 0;
    while (!stack.Empty()) {
        int res;
        stack.Pop(&res);
        std::cout << res << ' ';
    }
    std::cout << std::endl;
    return 0;
}
