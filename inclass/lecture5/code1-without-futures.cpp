#include <iostream>
#include <thread>
#include <mutex>

int result = 0; std::mutex m;

void compute() {
    int r = 42*42;
    std::lock_guard<std::mutex> lg(m);
    result = r;
}

int main() {
    std::thread t(compute);
    t.join();
    std::lock_guard<std::mutex> lg(m);
    std::cout << "Result: " << result << "\n";
}
