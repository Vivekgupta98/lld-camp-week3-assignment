#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#include <stdexcept>
#include <iostream>

class ThreadPool {
public:
    explicit ThreadPool(std::size_t n = std::thread::hardware_concurrency())
    : stop_(false) {
        if (!n) n = 1;
        workers_.reserve(n);
        for (std::size_t i = 0; i < n; ++i) {
            workers_.emplace_back([this] {
                for (;;) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lk(m_);
                        cv_.wait(lk, [this]{ return stop_ || !q_.empty(); });
                        if (stop_ && q_.empty()) return;
                        task = std::move(q_.front());
                        q_.pop();
                    }
                    try { task(); } catch (...) { /* swallow/log */ }
                }
            });
        }
    }

    // Fire-and-forget: just ignore the returned future
    template<class F, class... A>
    void enqueue(F&& f, A&&... a) {
        (void)submit(std::forward<F>(f), std::forward<A>(a)...);
    }

    // Submit and get a future result
    template<class F, class... A>
    auto submit(F&& f, A&&... a)
      -> std::future<typename std::result_of<F(A...)>::type>
    {
        using R = typename std::result_of<F(A...)>::type;
        auto pkg = std::make_shared<std::packaged_task<R()>>(
            std::bind(std::forward<F>(f), std::forward<A>(a)...));
        auto fut = pkg->get_future();
        {
            std::lock_guard<std::mutex> lk(m_);
            if (stop_) throw std::runtime_error("submit on stopped pool");
            q_.emplace([pkg]{ (*pkg)(); });
        }
        cv_.notify_one();
        return fut;
    }

    ~ThreadPool() {
        {
            std::lock_guard<std::mutex> lk(m_);
            stop_ = true;
        }
        cv_.notify_all();
        for (auto& t : workers_) if (t.joinable()) t.join();
    }

private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> q_;
    std::mutex m_;
    std::condition_variable cv_;
    bool stop_;
};

// --- demo ---
int add(int a, int b) { return a + b; }

int main() {
    ThreadPool pool(4);

    for (int i = 0; i < 6; ++i)
        pool.enqueue([i]{
            std::cout << "Task " << i
                      << " on thread " << std::this_thread::get_id() << "\n";
        });

    auto f1 = pool.submit(add, 21, 21);
    auto f2 = pool.submit([](int x){ return x * x; }, 12);

    std::cout << "add: " << f1.get() << "\n";   // 42
    std::cout << "sq : " << f2.get() << "\n";   // 144
}
