#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>
#include <iostream>

// ---------------- TSQueue (thread-safe, closeable) ----------------
template <class T>
class TSQueue {
public:
    TSQueue() : closed_(false) {}

    TSQueue(const TSQueue&) = delete;
    TSQueue& operator=(const TSQueue&) = delete;

    // Producers
    bool push(const T& v) {
        std::lock_guard<std::mutex> lk(m_);
        if (closed_) return false;
        q_.push(v);
        cv_.notify_one();
        return true;
    }
    bool push(T&& v) {
        std::lock_guard<std::mutex> lk(m_);
        if (closed_) return false;
        q_.push(std::move(v));
        cv_.notify_one();
        return true;
    }
    template<class... Args>
    bool emplace(Args&&... args) {
        std::lock_guard<std::mutex> lk(m_);
        if (closed_) return false;
        q_.emplace(std::forward<Args>(args)...);
        cv_.notify_one();
        return true;
    }

    // Consumers
    bool try_pop(T& out) {
        std::lock_guard<std::mutex> lk(m_);
        if (q_.empty()) return false;
        out = std::move(q_.front());
        q_.pop();
        return true;
    }

    // Blocks until item available OR queue is closed & drained.
    // Returns false only when closed and empty.
    bool wait_pop(T& out) {
        std::unique_lock<std::mutex> lk(m_);
        cv_.wait(lk, [this]{ return closed_ || !q_.empty(); });
        if (q_.empty()) return false; // closed and drained
        out = std::move(q_.front());
        q_.pop();
        return true;
    }

    // Shutdown producers and wake all consumers.
    void close() {
        std::lock_guard<std::mutex> lk(m_);
        closed_ = true;
        cv_.notify_all();
    }

    bool closed() const {
        std::lock_guard<std::mutex> lk(m_);
        return closed_;
    }

    bool empty() const {
        std::lock_guard<std::mutex> lk(m_);
        return q_.empty();
    }

    std::size_t size() const {
        std::lock_guard<std::mutex> lk(m_);
        return q_.size();
    }

private:
    mutable std::mutex m_;
    std::condition_variable cv_;
    std::queue<T> q_;
    bool closed_;
};

// ---------------- ThreadPool using TSQueue ----------------
class ThreadPool {
public:
    explicit ThreadPool(std::size_t n = std::thread::hardware_concurrency()) {
        if (!n) n = 1;
        workers_.reserve(n);
        for (std::size_t i = 0; i < n; ++i) {
            workers_.emplace_back([this]{
                std::function<void()> task;
                while (tasks_.wait_pop(task)) {
                    try { task(); } catch (...) { /* swallow/log */ }
                }
            });
        }
    }

    // Fire-and-forget
    template<class F, class... A>
    void enqueue(F&& f, A&&... a) {
        (void)submit(std::forward<F>(f), std::forward<A>(a)...);
    }

    // Submit and get future
    template<class F, class... A>
    auto submit(F&& f, A&&... a)
      -> std::future<typename std::result_of<F(A...)>::type>
    {
        using R = typename std::result_of<F(A...)>::type;
        auto pkg = std::make_shared<std::packaged_task<R()>>(
            std::bind(std::forward<F>(f), std::forward<A>(a)...));
        auto fut = pkg->get_future();
        if (!tasks_.emplace([pkg]{ (*pkg)(); })) {
            throw std::runtime_error("submit on stopped pool");
        }
        return fut;
    }

    // Optional explicit stop if you want to end before destruction.
    void shutdown() {
        tasks_.close();
    }

    ~ThreadPool() {
        tasks_.close(); // signal shutdown
        for (auto& t : workers_) if (t.joinable()) t.join();
    }

private:
    TSQueue<std::function<void()>> tasks_;
    std::vector<std::thread> workers_;
};

// ---------------- Demo ----------------
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

    // pool.shutdown(); // optional early stop before destructor
}
