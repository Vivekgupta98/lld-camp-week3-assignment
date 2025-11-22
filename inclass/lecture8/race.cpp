
// race.cpp
// Build (Linux/Clang/GCC):
//   g++ -std=c++14 -fsanitize=thread -g race.cpp -o race
// Run:
//   ./race
//
// Toggle fix:
//   g++ -std=c++14 -fsanitize=thread -g -DUSE_FIX race.cpp -o race


#include <atomic>
#include <chrono>
#include <iostream>
#include <random>
#include <thread>

constexpr int ITER = 500000;

#ifndef USE_FIX
int counter = 0;                       // racy
#else
std::atomic<int> counter{0};           // fixed
#endif

void tiny_pause() {
    static thread_local std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<int> dist(0, 50);
    std::this_thread::sleep_for(std::chrono::nanoseconds(dist(rng)));
}

void worker(int id) {
    for (int i = 0; i < ITER; ++i) {
#ifndef USE_FIX
        counter++;                     // data race here
#else
        counter.fetch_add(1, std::memory_order_relaxed);
#endif
        if ((i & 0x7FFF) == 0) tiny_pause();
    }
    if ((id & 1) == 0) tiny_pause();
}

int main() {
    std::cout << "Starting race demo (ITER=" << ITER
#ifndef USE_FIX
              << ", RACY"
#else
              << ", FIXED"
#endif
              << ")\n";

    std::thread t1(worker, 1);
    std::thread t2(worker, 2);
    t1.join();
    t2.join();

    std::cout << "Final counter = " << counter
              << " (expected " << (2 * ITER) << ")\n";
}
