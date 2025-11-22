// capture_by_ref.cpp
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    int x = 10;

    std::thread t1([&]() { // Change to = to capture by value
        // std::this_thread::sleep_for(std::chrono::milliseconds(50));
        std::cout << "[worker] sees x = " << x << "\n"; // reads by reference
        x += 5; // modifies the original x
    });

     std::thread t2([&]() { // Change to = to capture by value
        // std::this_thread::sleep_for(std::chrono::milliseconds(50));
        std::cout << "[worker] sees x = " << x << "\n"; // reads by reference
        x += 5; // modifies the original x
    });

    // std::this_thread::sleep_for(std::chrono::milliseconds(50));
    // Modify x while worker is sleeping
    x = 42;

    t1.detach(); 
    t2.join();
    // std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    std::cout << "[main] final x = " << x << " (worker may have added +5)\n";
    return 0;

}
// Run : g++ -std=c++17 -pthread lecture2/code7-lambdaref.cpp -o code7-lambdaref && ./code7-lambdaref