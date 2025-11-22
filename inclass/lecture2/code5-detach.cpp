// demo_detach.cpp
#include <iostream>
#include <thread>
#include <chrono>
#include <random>

int main() {
    std::thread t([]{
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        std::cout << "[bg] background task finished\n";
    });

    if (t.joinable()) t.detach(); // now runs independently

    // Random small delay to make outcomes vary across runs
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> jitter(0, 250);
    int ms = jitter(gen);

    std::cout << "[main] exiting in ~" << ms << " ms\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    std::cout << "[main] exit now\n";
    return 0; // process may end before detached thread prints
}


