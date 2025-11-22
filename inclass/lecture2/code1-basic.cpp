#include <iostream>
#include <thread>
void worker(int n) { 
    std::cout << "Worker: " << n << "\n";
}

int main() {
    std::thread t(worker, 5);
    t.join();
    std::cout << "Main finished\n";
    return 0;
}

// Run : g++ -std=c++17 -pthread lecture2/code1-basic.cpp -o code1-basic && ./code1-basic