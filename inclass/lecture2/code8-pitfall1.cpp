// #include <iostream>
// #include <thread>
// #include <chrono>
// #include <string>

// int main() {
//     std::thread t([&]() {
//         // Captures everything by reference — including 'msg'
//         // Thread will use 'msg' after it has gone out of scope => UB
//         std::this_thread::sleep_for(std::chrono::milliseconds(150));
//         // Using a dangling reference may crash or print garbage:
//         // std::cout << "[worker] msg = " << msg << "\n"; // UB
//     });

//     std::string msg = "hello, world";
//     // OOPS: detach + local variable + reference capture = danger
//     t.detach();

//     // 'msg' goes out of scope soon; worker might still use it after this line
//     std::cout << "[main] exiting; worker may still be running...\n";
//     return 0; // Process ends or 'msg' dies before worker uses it => UB
// }
// RUN : g++ -std=c++17 -pthread lecture2/code8-pitfall1.cpp -o code8-pitfall1 && ./code8-pitfall1


// shared_ptr_fix.cpp
#include <iostream>
#include <thread>
#include <chrono>
#include <memory>
#include <string>

int main() {
    auto msg = std::make_shared<std::string>("hello, world");

    std::thread t([sp = msg]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
        std::cout << "[worker] msg = " << *sp << "\n"; // safe: shared ownership
    });

    t.detach(); // okay: the object stays alive until the worker releases the last ref
    msg.reset(); // main can drop its reference early

    std::cout << "[main] dropped my ref; worker still has it safely\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(250)); // let worker finish
    std::cout << "[main] exit\n";
    return 0;
}
