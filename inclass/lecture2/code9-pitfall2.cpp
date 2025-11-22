#include <iostream>
#include <thread>
#include <vector>
#include <chrono>

int main() {
    std::vector<int> big(1'000'000, 42);
    int i = 7;

    // ❌ Over-capturing by value: copies the whole 'big' vector into the lambda
    auto heavy = std::thread([=]() {
        std::cout << "[heavy] size = " << big.size() << ", i = " << i << "\n";
    });

    // ✅ Precise capture: only capture what you need (by value or ref)
    auto light = std::thread([i]() { // don't pull in 'big' at all
        std::cout << "[light] i = " << i << "\n";
    });

    heavy.join();
    light.join();
    return 0;
}
// over_capturing.cpp
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>

int main() {
    std::vector<int> big(1'000'000, 42);
    int i = 7;

    // ❌ Over-capturing by value: copies the whole 'big' vector into the lambda
    auto heavy = std::thread([=]() {
        std::cout << "[heavy] size = " << big.size() << ", i = " << i << "\n";
    });

    // ✅ Precise capture: only capture what you need (by value or ref)
    auto light = std::thread([i]() { // don't pull in 'big' at all
        std::cout << "[light] i = " << i << "\n";
    });

    heavy.join();
    light.join();
    return 0;
}

//Fix: Prefer explicit captures like [i], [&vec], or [ptr=...] to avoid copying large state.
