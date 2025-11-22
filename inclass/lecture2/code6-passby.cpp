#include <iostream>
#include <thread>
#include <functional>
void increment(int &x) { x++; }

int main() {
    int counter = 0;
    std::thread t(increment,std::ref(counter)); // counter passed by reference
    t.join();
    std::cout << counter << "\n";
}
// Run : g++ -std=c++17 -pthread lecture2/code6-passby.cpp -o code6-passby && ./code6-passby