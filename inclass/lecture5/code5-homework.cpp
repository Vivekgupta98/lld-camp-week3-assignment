// async_sum_any_order.cpp
#include <future>
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>

int compute(int id) {
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(50ms * (6 - id)); // different durations
    return id * 10;
}

int main() {
    using namespace std::chrono;
    using namespace std::chrono_literals;

    // Force asynchronous launch (avoid deferred execution)
    std::vector<std::future<int>> futs;
    for (int i = 1; i <= 5; ++i) {
        futs.emplace_back(std::async(std::launch::async, compute, i));
    }

    int sum = 0;
    std::vector<bool> done(futs.size(), false);
    std::size_t remaining = futs.size();

    // Collect results as soon as each future becomes ready
    while (remaining > 0) {
        bool progressed = false;

        for (std::size_t i = 0; i < futs.size(); ++i) {
            if (done[i]) continue;

            if (futs[i].wait_for(0ms) == std::future_status::ready) {
                sum += futs[i].get();    // get() once; makes future invalid
                done[i] = true;
                --remaining;
                progressed = true;
            }
        }

        if (!progressed) {
            // Back off briefly to avoid busy-waiting
            std::this_thread::sleep_for(1ms);
        }
    }

    std::cout << "Sum = " << sum << "\n";
    return 0;
}
