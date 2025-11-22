// all_in_one_lock_tags.cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <queue>
#include <chrono>

// Demo 1: defer_lock
std::mutex g_m_defer;
int g_shared_value = 0;

// Demo 2: try_to_lock
std::mutex g_m_try;
int g_guarded = 0;

// Demo 3: adopt_lock (two mutexes)
std::mutex g_mA, g_mB;
int g_a = 0, g_b = 0;

// ---------- Demo 1: defer_lock ----------
void worker_defer(int id) {
    // Pretend prep work (no lock held)
    int local = id * 10;

    // Construct lock object without locking yet
    std::unique_lock<std::mutex> lk(g_m_defer, std::defer_lock);

    // ... maybe more prep ...

    lk.lock(); // enter critical section exactly when needed
    g_shared_value += local;
    std::cout << "[defer] T" << id
              << " added " << local
              << " -> shared=" << g_shared_value << "\n";

    // Early release to reduce contention
    lk.unlock();

    // Post-processing (no lock held)
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
}

void demo_defer_lock() {
    std::cout << "\n=== Demo: std::defer_lock ===\n";
    std::vector<std::thread> ts;
    for (int i = 1; i <= 4; ++i) ts.emplace_back(worker_defer, i);
    for (auto& t : ts) t.join();
    std::cout << "[defer] Final shared = " << g_shared_value << "\n";
}

// ---------- Demo 2: try_to_lock ----------
void fast_try_worker(int id) {
    // Non-blocking attempt to acquire the lock
    std::unique_lock<std::mutex> lk(g_m_try, std::try_to_lock);
    if (lk.owns_lock()) {
        g_guarded += id;
        std::cout << "[try] T" << id << " acquired; guarded=" << g_guarded << "\n";
        // lock auto-released when lk goes out of scope
    } else {
        std::cout << "[try] T" << id << " busy; doing fallback work\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}

void demo_try_to_lock() {
    std::cout << "\n=== Demo: std::try_to_lock ===\n";

    // Hold the lock briefly to force some immediate try-lock failures
    std::thread blocker([] {
        std::unique_lock<std::mutex> lk(g_m_try);
        std::cout << "[try:blocker] holding lock...\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(60));
        std::cout << "[try:blocker] releasing lock.\n";
    });

    // Give blocker a head start
    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    std::thread t1(fast_try_worker, 1);
    std::thread t2(fast_try_worker, 2);
    std::thread t3(fast_try_worker, 3);

    blocker.join();
    t1.join(); t2.join(); t3.join();
    std::cout << "[try] Final guarded = " << g_guarded << "\n";
}

// ---------- Demo 3: adopt_lock ----------
void update_both(int id) {
    // Lock both without deadlock
    std::lock(g_mA, g_mB);

    // Adopt: assume already locked; guards will only OWN + unlock on destruction
    std::unique_lock<std::mutex> lkA(g_mA, std::adopt_lock);
    std::unique_lock<std::mutex> lkB(g_mB, std::adopt_lock);

    g_a += id;
    g_b += id * 2;
    std::cout << "[adopt] T" << id << " updated (a=" << g_a << ", b=" << g_b << ")\n";
    // lkA/lkB destructors unlock automatically
}

void demo_adopt_lock() {
    std::cout << "\n=== Demo: std::adopt_lock (+ std::lock) ===\n";
    std::vector<std::thread> ts;
    for (int i = 1; i <= 4; ++i) ts.emplace_back(update_both, i);
    for (auto& t : ts) t.join();
    std::cout << "[adopt] Final: a=" << g_a << ", b=" << g_b << "\n";
}

// ---------- main ----------
int main() {
    demo_defer_lock();
    demo_try_to_lock();
    demo_adopt_lock();
    std::cout << "\nAll demos complete.\n";
    return 0;
}
