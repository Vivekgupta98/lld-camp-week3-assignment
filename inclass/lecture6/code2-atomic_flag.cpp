#include <atomic>
#include <thread>
#include <iostream>

std::atomic_flag lock = ATOMIC_FLAG_INIT;
int shared_value = 0;

void work() {
    while (lock.test_and_set(std::memory_order_acquire)) { /* spin */ }
    ++shared_value;
    lock.clear(std::memory_order_release);
}

int main() {
    std::thread t1(work), t2(work);
    t1.join(); t2.join();
    std::cout << shared_value << "\n";
}
