#include <future>
#include <iostream>

int compute() { return 42; }

int main() {
    std::future<int> f = std::async(std::launch::async, compute);
    std::cout << "Result: " << f.get() << "\n";
}
