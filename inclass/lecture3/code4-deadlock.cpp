#include <thread>
#include <mutex>
#include <chrono>
#include <iostream>

std::mutex m1, m2;

void f() {
    std::lock_guard<std::mutex> g1(m1);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::lock_guard<std::mutex> g2(m2);
    std::cout << "f done\n";
}

void g() {
    std::lock_guard<std::mutex> g1(m1);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::lock_guard<std::mutex> g2(m2);
    std::cout << "g done\n";
}

int main() {
    std::thread t1(f), t2(g);
    t1.join(); t2.join();
    return 0;
}
// Run : g++ -std=c++17 -pthread lecture3/code4-deadlock.cpp -o code4-deadlock && ./code4-deadlock