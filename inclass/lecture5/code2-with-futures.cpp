#include <future>
#include <iostream>
#include <thread>

void compute(std::promise<int> p) {
    try {
        int r = 42;
        p.set_value(r);            // or p.set_exception(std::current_exception());
    } catch (...) { p.set_exception(std::current_exception()); }
}

int main() {
    std::promise<int> p;
    std::future<int> f = p.get_future();
    std::thread t(compute, std::move(p));

    int result = f.get();          // blocks until set_value/exception
    std::cout << "Result: " << result << "\n";
    t.join();
}
