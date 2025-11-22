#include <future>
#include <thread>
#include <iostream>

int add(int a, int b) { return a + b; }

int main() {
    std::packaged_task<int(int,int)> task(add);
    std::future<int> fut = task.get_future();

    std::thread worker(std::move(task), 5, 7); // task invoked here
    std::cout << "Sum: " << fut.get() << "\n";
    worker.join();
}

