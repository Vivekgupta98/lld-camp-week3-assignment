#include <atomic>
#include <thread>
#include <iostream>

std::atomic<bool> ready{false};
int data = 0;

void producer() {
    data = 42;
    ready.store(true, std::memory_order_release);
}
void consumer() {
    while (!ready.load(std::memory_order_acquire)) {}
    std::cout << "Data = " << data << "\n"; // guaranteed 42
}

int main() {
    std::thread t1(producer), t2(consumer);
    t1.join(); t2.join();
}
