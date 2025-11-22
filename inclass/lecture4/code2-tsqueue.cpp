#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#include <chrono>

class ThreadSafeQueue {
public:
    void push(int x) {
        {
            std::lock_guard<std::mutex> lk(mu_);
            q_.push(x);
        }
        cv_.notify_one();
    }

    int pop() {
        std::unique_lock<std::mutex> lk(mu_);
        cv_.wait(lk, [this]{ return !q_.empty(); });
        int v = q_.front(); q_.pop();
        return v;
    }

private:
    std::queue<int> q_;
    std::mutex mu_;
    std::condition_variable cv_;
};

int main() {
    ThreadSafeQueue tsq;

    std::thread producer([&]{
        for (int i = 1; i <= 10; ++i) {
            tsq.push(i);
            std::cout << "Produced " << i << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    });

    std::thread consumer([&]{
        for (int i = 1; i <= 10; ++i) {
            int v = tsq.pop();
            std::cout << "Consumed " << v << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(80));
        }
    });

    producer.join();
    consumer.join();
}
