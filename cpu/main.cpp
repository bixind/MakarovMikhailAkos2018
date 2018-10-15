#include "parser.h"
#include <iostream>
#include <fstream>
#include <sstream>

int main() {
    std::stringstream buffer;
    std::streambuf * old = std::cout.rdbuf(buffer.rdbuf());

    std::cout << 123 << std::endl;
    std::cin.rdbuf(buffer.rdbuf());
    int a = 0;
    std::cin >> a;
    std::cout.rdbuf(old);
    std::cout << buffer.str().size() << std::endl;
    std::cout << a << std::endl;

}