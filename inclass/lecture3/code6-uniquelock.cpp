// unique_lock_defer_unlock.cpp
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
#include <chrono>

std::mutex m;
int shared = 0;

void worker(int id) {
    // Do prep work without holding the lock
    int local = id * 10; // pretend this is expensive

    std::unique_lock<std::mutex> lk(m, std::defer_lock); // not locked yet
    // ... maybe more prep ...
    lk.lock(); // enter critical section exactly when needed

    shared += local;
    std::cout << "T" << id << " added " << local << ", shared=" << shared << "\n";

    // Done with shared state: release early to shorten contention
    lk.unlock();

    // Post-processing (no lock held)
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

int main() {
    std::vector<std::thread> ts;
    for (int i = 1; i <= 4; ++i) ts.emplace_back(worker, i);
    for (auto& t : ts) t.join();
    std::cout << "Final shared = " << shared << "\n";
}
