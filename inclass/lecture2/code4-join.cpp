#include <iostream>
#include <thread>
#include <chrono>

void work() {
    std::cout << "[worker] start on " << std::this_thread::get_id() << "\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    std::cout << "[worker] done\n";
}

int main() {
    std::thread t(work);
    std::cout << "joinable before join? " << std::boolalpha << t.joinable() << "\n";

    t.join(); // main waits here

    std::cout << "joinable after join?  " << std::boolalpha << t.joinable() << "\n";
    std::cout << "[main] finished safely\n";
    return 0;
}

// Run : g++ -std=c++17 -pthread lecture2/code4-join.cpp -o code4-join && ./code4-join