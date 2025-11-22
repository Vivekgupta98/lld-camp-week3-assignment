#include <iostream>
#include <thread>
#include <chrono>

using namespace std::chrono_literals;

void task(const char* name) {
    // Do a few prints with tiny pauses so interleaving is obvious
    for (int i = 1; i <= 3; ++i) {
        std::cout << name << " says hello " << i << std::endl;
        std::this_thread::sleep_for(10ms); // nudge scheduler, not required
    }
}

int main() {
    std::thread t1(task, "Task 1");
    std::thread t2(task, "Task 2");

    t1.join();
    t2.join();

    std::cout << "Done.\n";
    return 0;
}
