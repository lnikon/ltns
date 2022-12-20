#include <iostream>
#include <utility>

int main() {
    std::pair<std::string, double> pair {"Hello, World!", 42};

    const auto& [str, num] = pair;
    std::cout << str << ": " << num << std::endl;

    return 0;
}
