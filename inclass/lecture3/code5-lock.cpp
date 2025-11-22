#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

std::mutex m1, m2;
int data1 = 0, data2 = 0;

void task(int id) {
    // Atomically acquire both mutexes without deadlock
    std::lock(m1, m2);
    std::lock_guard<std::mutex> g1(m1, std::adopt_lock);
    std::lock_guard<std::mutex> g2(m2, std::adopt_lock);

    // Critical section touching both protected objects
    data1 += id;
    data2 += id * 2;
    std::cout << "T" << id << " updated: (" << data1 << ", " << data2 << ")\n";
}

int main() {
    std::vector<std::thread> ts;
    for (int i = 1; i <= 4; ++i) ts.emplace_back(task, i);
    for (auto& t : ts) t.join();
    std::cout << "Final: (" << data1 << ", " << data2 << ")\n";
}
