#include <atomic>
#include <thread>
#include <iostream>

std::atomic<int> x{0}, y{0};
int r1, r2;

int main() {
    // Change the memory_order to see different results

    std::thread t1([] {
        x.store(1);              // (A)
        r1 = y.load();           // (B)
    });

    std::thread t2([] {
        y.store(1);              // (C)
        r2 = x.load();           // (D)
    });

    t1.join();
    t2.join();

    std::cout << r1 << " " << r2 << "\n";
}
