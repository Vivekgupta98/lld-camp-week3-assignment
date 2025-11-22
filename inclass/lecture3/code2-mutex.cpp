#include <iostream>
#include <thread>
#include <mutex>

int counter = 0;
std::mutex m;

void increment() {
    for (int i = 0; i < 1000; ++i) {
        m.lock();
        ++counter;
        m.unlock();
    }
}


int main() {
    std::thread t1(increment), t2(increment);
    t1.join(); t2.join();
    std::cout << "Final: " << counter << "\n"; // often < 2000
}
// Run : g++ -std=c++17 -pthread lecture3/code2-mutex.cpp -o code2-mutex && ./code2-mutex