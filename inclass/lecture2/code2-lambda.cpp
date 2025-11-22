#include <iostream>
#include <thread>

int main() {
    int x = 5;

    // Launch a thread with a lambda that captures x by value
    std::thread t([x]() {
        std::cout << "Worker (lambda): " << x << "\n";
    });

    t.join();
    std::cout << "Main finished\n";
    return 0;
}