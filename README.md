# Concurrent Job Processing System  
_A multi-phase project to learn threads, mutexes, condition variables & futures in C++_

---

## 1. Project Overview

In this project, you will build a **concurrent job processing engine** in C++.

Imagine a backend service where many clients submit tasks like:

- “Compute sum from 1 to 10 million”
- “Count primes between 1 and 1 million”
- “Do some long-running job that might fail”

Instead of running everything in a single thread, we:

- Put jobs into a **shared queue**,
- Have multiple **worker threads** process them in parallel,
- Let the caller **wait for results** using **futures**,
- Support **graceful shutdown** and **basic metrics**.

You will build this step-by-step in phases.  
Each phase introduces specific concurrency concepts (threads, mutex, condition variables, promises/futures, atomics, deadlock).

---

## 2. Learning Goals

By the end of this project, you should be comfortable with:

- Creating and joining threads with `std::thread`
- Protecting shared data using `std::mutex`, `std::lock_guard`, `std::unique_lock`
- Using `std::condition_variable` for producer–consumer patterns
- Using `std::promise`, `std::future`, and `std::packaged_task`
- Using `std::atomic` for simple, lock-free counters & flags
- Understanding & avoiding deadlocks (lock ordering / `std::scoped_lock`)
- Designing and using a basic **thread pool** abstraction

---

## 3. Prerequisites

You should already know:

- Basic C++ (classes, references, RAII, exceptions)
- STL containers (`std::vector`, `std::queue`, `std::map`)
- Basic OOP concepts (interfaces via abstract classes)

You are expected to use **C++17 or later**.

---

## 4. Setup & Requirements

### 4.1. Tools

- Compiler: `g++` or `clang++` with C++17 support  
  Example compile command:
  ```bash
  g++ -std=gnu++17 -O2 -pthread main.cpp -o concurrent_jobs
  ```

- OS: Linux / macOS / WSL (Windows Subsystem for Linux) recommended

### 4.2. Code Structure (Suggested)

You are free to structure files as you like, but this is a suggested layout:

- `Job.hpp` – Job interface & JobResult
- `Jobs.hpp` – Concrete job types
- `SingleThreadJobEngine.hpp` – Phase 1 engine
- `BlockingJobQueue.hpp` – Shared queue with mutex + condition variable
- `ThreadPool.hpp` – Thread pool implementation
- `main.cpp` – Demo / tests for each phase

You can also keep everything in a single file if you prefer, but good separation will help.

---

## 5. Project Phases

You must implement the project in **phases**.  
Each phase builds on previous ones. Do not skip phases.

### Phase 1 – Basic Jobs & Single-Threaded Engine

**Concept Focus:** Clean design, result handling, no concurrency yet.

#### Task

1. Define a result type:

   ```cpp
   struct JobResult {
       bool success;
       std::string message;  // e.g. "OK" or "Failed: <reason>"
       int value;            // example numeric output
   };
   ```

2. Define an abstract job:

   ```cpp
   class Job {
   public:
       virtual ~Job() = default;
       virtual JobResult run() = 0;
   };
   ```

3. Implement at least three concrete jobs:
   - `SumRangeJob(int l, int r)` – computes sum from `l` to `r`.
   - `PrimeCountJob(int l, int r)` – counts primes in `[l, r]`.
   - `FailingJob()` – throws a `std::runtime_error` in `run()` to simulate failure.

4. Implement a **single-threaded engine**:

   ```cpp
   class SingleThreadJobEngine {
   public:
       JobResult executeNow(Job& job);  // calls job.run()
   };
   ```

5. In `main.cpp`:
   - Create instances of the jobs.
   - Execute them using `SingleThreadJobEngine`.
   - Print out `success`, `message`, and `value`.

**Checklist:**

- [ ] Can you execute different jobs via same `Job*` interface?
- [ ] Does `FailingJob` show how exceptions might be handled later?

---

### Phase 2 – Shared Queue with Mutex (No Condition Variable Yet)

**Concept Focus:** Shared data structures + `std::mutex`.

#### Task

1. Implement a simple **thread-safe job queue**:

   ```cpp
   class JobQueue {
   public:
       void push(std::unique_ptr<Job> job);
       std::unique_ptr<Job> tryPop(); // returns nullptr if empty

   private:
       std::mutex m;
       std::queue<std::unique_ptr<Job>> q;
   };
   ```

2. Implement:
   - `push`: lock, push, unlock (using `std::lock_guard<std::mutex>`).
   - `tryPop`: lock, check if empty, if not empty pop & return, otherwise return `nullptr`.

3. In `main.cpp`, create:
   - One **producer thread** that pushes ~100 jobs into the queue.
   - The **main thread** repeatedly calls `tryPop()` in a loop:
     - If a job is obtained, execute it via `SingleThreadJobEngine`.
     - If `nullptr`, sleep for a short duration (`std::this_thread::sleep_for`) to avoid busy-spinning.

**Checklist:**

- [ ] Did you protect the queue with a mutex?
- [ ] Did you see that this design may still waste CPU (busy-waiting)?

---

### Phase 3 – Blocking Queue with Condition Variable

**Concept Focus:** `std::condition_variable`, blocking producer–consumer.

#### Task

1. Upgrade your queue to a **blocking queue**:

   ```cpp
   class BlockingJobQueue {
   public:
       void push(std::unique_ptr<Job> job);
       std::unique_ptr<Job> pop();   // blocks until job or shutdown
       void shutdown();              // signal no more jobs

   private:
       std::mutex m;
       std::condition_variable cv;
       std::queue<std::unique_ptr<Job>> q;
       bool stopped = false;
   };
   ```

2. Behavior:

   - `push`:
     - Lock mutex, push job, unlock, then `cv.notify_one()`.

   - `pop`:
     - Acquire `std::unique_lock<std::mutex>`.
     - Call `cv.wait(lock, [&]{ return !q.empty() || stopped; });`
     - If `stopped` and `q.empty()`, return `nullptr` (no more work).
     - Otherwise, pop and return one job.

   - `shutdown`:
     - Lock, set `stopped = true`, unlock, `cv.notify_all()`.

3. Demo scenario:

   - Start **2 producer threads** that each push a bunch of jobs.
   - Start **1 consumer thread** that:
     - Calls `pop()` in a loop.
     - If `nullptr` is returned, exits.
     - Otherwise executes the job and prints the result.
   - When producers are done, call `shutdown()` and join the consumer.

**Checklist:**

- [ ] Did you use `cv.wait` with a lambda predicate?
- [ ] Do your threads exit cleanly without hanging?

---

### Phase 4 – Thread Pool with Futures

**Concept Focus:** `std::future`, `std::promise` / `std::packaged_task`, thread pool pattern.

#### Task

1. Implement a **ThreadPool**:

   ```cpp
   class ThreadPool {
   public:
       explicit ThreadPool(size_t numWorkers);
       ~ThreadPool();

       template <typename F>
       std::future<JobResult> submit(F&& jobFunc);

       void shutdown();  // optional explicit shutdown

   private:
       // worker threads
       std::vector<std::thread> workers;
       // queue of generic tasks
       // (You may need a separate BlockingTaskQueue that stores std::function<void()>)
       // and any additional flags/state.
   };
   ```

2. Internal design:

   - Let the queue contain:
     ```cpp
     using Task = std::function<void()>;
     ```
   - Each worker thread runs:
     ```cpp
     void workerLoop() {
         for (;;) {
             auto task = taskQueue.pop(); // returns empty / null when shutdown
             if (!task) break;
             (*task)();
         }
     }
     ```

3. Implement `submit`:

   - Take a callable `F jobFunc` that returns `JobResult`.
   - Create a `std::packaged_task<JobResult()>` from it.
   - Get its `std::future<JobResult>`.
   - Wrap the packaged_task into a `Task` that, when called, invokes the packaged_task.
   - Push this `Task` into the queue.
   - Return the future to the caller.

4. Implement destructor / `shutdown`:

   - Tell the queue to shut down (e.g., via `shutdown()`).
   - Join all worker threads.

5. In `main.cpp`:

   - Create a `ThreadPool` with `4` workers.
   - Submit 50+ jobs (mix of `SumRangeJob`, `PrimeCountJob`, and failures).
   - Store futures in a vector.
   - Later, iterate and call `get()` on each future:
     - Print results or error messages (catch exceptions if you propagate them).

**Checklist:**

- [ ] Can you submit lambdas like `[=](){ return sumRangeJob.run(); }`?
- [ ] Do all futures eventually become ready?
- [ ] Does the program exit cleanly with all threads joined?

---

### Phase 5 – Atomics & Deadlock Side Quest

This part is partially optional but highly recommended.

#### 5A. Metrics with `std::atomic`

**Concept Focus:** Simple lock-free counters with `std::atomic`.

1. Add metrics in `ThreadPool`:

   ```cpp
   std::atomic<int> tasksSubmitted{0};
   std::atomic<int> tasksCompleted{0};
   ```

2. In `submit`, increment `tasksSubmitted`.
3. After each task runs in a worker, increment `tasksCompleted`.
4. Provide getters / print these values periodically from `main`.

**Goals:**

- Show how `std::atomic` avoids the need for a mutex for trivial counters.
- Observe values increasing as work runs.

#### 5B. Deadlock Demo & Fix

**Concept Focus:** Deadlock, lock ordering, `std::scoped_lock`.

1. Create two resources protected by separate mutexes:

   ```cpp
   struct Resource {
       std::string name;
       std::mutex m;
       // maybe some payload
   };
   ```

2. Start two threads:

   - Thread 1: locks A then B.
   - Thread 2: locks B then A.

3. Under some runs, your program may **deadlock**.

4. Fix it by either:

   - Using a **consistent lock order** (always lock A then B in both threads), or
   - Using `std::scoped_lock(A.m, B.m);` (C++17).

**Goal:** Experience how deadlocks are created and how to systematically avoid them.

---

### Phase 6 – Graceful Shutdown & (Optional) Cooperative Cancellation

**Concept Focus:** Stop flags, clean shutdown of long-running tasks.

1. Add a **stop flag** to the pool:

   ```cpp
   std::atomic<bool> stopRequested{false};
   ```

2. In `shutdown()`, set `stopRequested = true` and shut down the queue.

3. Create a “long-running” job (e.g., loops with `sleep_for`):

   - Periodically check some `std::atomic<bool> cancelled` (or a stop token if you use C++20).
   - Exit early if cancellation is requested.

4. Demo:

   - Submit several long-running jobs.
   - After a while, call `shutdown()`.
   - Make some of the jobs print that they were “cancelled” early.

**Goal:** Understand how “cancellation” is often **cooperative** in multithreaded systems.

---

## 6. What to Submit

You should submit:

1. **Source code**:
   - All `.cpp` / `.hpp` files.
   - Your `main.cpp` with demos for each phase.

2. **README additions** (or a short report) describing:
   - Which phases you completed.
   - Any design choices or deviations.
   - Known limitations or bugs.

3. (Optional) **Logs / screenshots** of runs that:
   - Show concurrent execution.
   - Show deadlock (before fix).
   - Show successful shutdown and metrics.

---

## 7. Tips & Suggestions

- Add **logging** with thread IDs:
  ```cpp
  std::cout << "[thread " << std::this_thread::get_id() << "] message\n";
  ```
- Start simple, get each phase working before moving on.
- Use `assert` or runtime checks to verify invariants (e.g., no negative counts).
- Never access shared data without a mutex unless it’s an `std::atomic` or read-only.

---

## 8. Optional Extensions (For Extra Challenge)

If you finish early, try:

- Adding **priority jobs** (high priority processed first).
- Adding a **timeout API**: `future.wait_for(...)` with custom behavior on timeout.
- Adding a simple web-like API simulation:
  - “Client threads” submit jobs, “server” is your thread pool.
- Measuring and comparing execution times:
  - Single-threaded vs 2 threads vs 4 threads vs 8 threads.

---

Good luck, and have fun breaking (and then fixing) your code with concurrency 🚀
