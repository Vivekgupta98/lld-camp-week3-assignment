#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <chrono>

std::queue<int> q;
bool done = false;

std::mutex mtx;
std::condition_variable cv;


void producer() {
    for (int i = 1; i <= 5; ++i) {
        {
            std::lock_guard<std::mutex> lk(mtx);
            q.push(i);
            std::cout << "Produced: " << i << "\n";
        }
        cv.notify_one(); // wake one waiting consumer
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    {
        std::lock_guard<std::mutex> lk(mtx);
        done = true;
    }
    cv.notify_all(); // so waiting consumers can exit
}

void consumer(int id) {
    while (true) {
        std::unique_lock<std::mutex> lk(mtx);
        cv.wait(lk, []{ return !q.empty() || done; });

        if (!q.empty()) {
            int item = q.front(); q.pop();
            lk.unlock();
            std::cout << "Consumer " << id << " got: " << item << "\n";
        } else if (done) {
            break;
        }
    }
}

int main() {
    std::thread p(producer);
    std::thread c1(consumer, 1), c2(consumer, 2);
    p.join(); c1.join(); c2.join();
}
