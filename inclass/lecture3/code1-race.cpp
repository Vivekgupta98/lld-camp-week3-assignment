#include <iostream>
#include <thread>

int counter = 0;

void increment() {
    for (int i = 0; i < 1000; ++i) counter++; // data race
}

int main() {
    std::thread t1(increment), t2(increment);
    t1.join(); t2.join();
    std::cout << "Final: " << counter << "\n"; // often < 2000
}
