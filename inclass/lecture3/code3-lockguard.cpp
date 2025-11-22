#include <iostream>
#include <thread>
#include <mutex>

int counter = 0;
std::mutex m;

void increment() {
    for (int i = 0; i < 1000; ++i) {
        std::lock_guard<std::mutex> lock(m);
        ++counter;                         // critical section
    }
}

int main() {
    std::thread t1(increment), t2(increment);
    t1.join(); t2.join();
    std::cout << "Final: " << counter << "\n"; // == 2000
}

// Run : g++ -std=c++17 -pthread lecture3/code3-lockguard.cpp -o code3-lockguard && ./code3-lockguard
