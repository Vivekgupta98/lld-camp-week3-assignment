// thread_ids.cpp
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>

int main() {
    // Main thread id and hardware concurrency hint
    std::cout << "Main thread id: " << std::this_thread::get_id() << "\n";
    unsigned hc = std::thread::hardware_concurrency();
    std::cout << "Hardware concurrency (hint): " << hc << "\n";

    // Launch a few worker threads and print their IDs
    const int N = 8;
    std::vector<std::thread> threads;
    threads.reserve(N);

    for (int i = 0; i < N; ++i) {
        threads.emplace_back([i]() {
            // Simulate a bit of work
            std::this_thread::sleep_for(std::chrono::milliseconds(100 * (i + 1)));
            std::cout << "Worker #" << i
                      << " thread id: " << std::this_thread::get_id() << "\n";
        });
    }

    for (auto& t : threads) t.join();

    std::cout << "All workers joined\n";
    return 0;
}

// Run : g++ -std=c++17 -pthread lecture2/code3-ids.cpp -o code3-ids && ./code3-ids