#include <iostream>
#include <thread>
#include <atomic>
#include <vector>

std::atomic<int> counter(0);

void increment() {
    for (int i = 0; i < 1000; ++i) {
        counter++; // atomic increment
        // Can use .fetch_add(1) as well
    }
}

int main() {
    std::vector<std::thread> ts;
    for (int i = 0; i < 10; ++i) ts.emplace_back(increment);
    for (auto& t : ts) t.join();
    std::cout << "Final counter: " << counter.load() << "\n"; // always 10000
}
