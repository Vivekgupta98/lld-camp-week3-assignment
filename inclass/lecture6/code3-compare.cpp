#include <atomic>
#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
using clock_type = std::chrono::high_resolution_clock;

static const int THREADS = 8;
static const int N = 2'000'000;

long long bench_mutex() {
    int counter = 0; std::mutex m;
    auto s = clock_type::now();
    std::vector<std::thread> ts;
    for (int t = 0; t < THREADS; ++t)
        ts.emplace_back([&]{
            for (int i = 0; i < N; ++i) { std::lock_guard<std::mutex> g(m); ++counter; }
        });
    for (auto& th : ts) th.join();
    auto e = clock_type::now();
    if (counter != THREADS * N) std::cerr << "wrong: " << counter << "\n";
    return std::chrono::duration_cast<std::chrono::milliseconds>(e - s).count();
}

long long bench_atomic() {
    std::atomic<int> counter{0};
    auto s = clock_type::now();
    std::vector<std::thread> ts;
    for (int t = 0; t < THREADS; ++t)
        ts.emplace_back([&]{ for (int i = 0; i < N; ++i) counter.fetch_add(1, std::memory_order_relaxed); });
    for (auto& th : ts) th.join();
    auto e = clock_type::now();
    if (counter.load() != THREADS * N) std::cerr << "wrong: " << counter.load() << "\n";
    return std::chrono::duration_cast<std::chrono::milliseconds>(e - s).count();
}

int main() {
    auto m = bench_mutex();
    auto a = bench_atomic();
    std::cout << "Mutex:  " << m << " ms\n";
    std::cout << "Atomic: " << a << " ms\n";
}
