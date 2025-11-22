// counter_race.cpp
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>

using namespace std::chrono_literals;

int counter = 0; // shared, NOT protected

void add_many(int n, const char* name) {
    for (int i = 0; i < n; ++i) {
        // Simulate work so the scheduler interleaves the threads
        counter++;
        if ((i % (n / 4 + 1)) == 0) { // occasional print to see interleaving
            std::cout << name << " incremented to ~" << counter << "\n";
        }
    }
}

int main() {
    const int per_thread = 100'000; // try raising this if your CPU is fast
    std::thread t1(add_many, per_thread, "T1");
    std::thread t2(add_many, per_thread, "T2");
    t1.join();
    t2.join();

    std::cout << "Expected: " << (2 * per_thread)
              << "\nActual:   " << counter << "\n";
}
