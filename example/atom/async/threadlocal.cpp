#include "atom/async/threadlocal.hpp"

#include <iostream>
#include <thread>

void threadFunction(atom::async::ThreadLocal<int>& threadLocal) {
    // Initialize thread-local value
    threadLocal.reset(42);
    std::cout << "Thread ID: " << std::this_thread::get_id()
              << ", Value: " << *threadLocal << std::endl;
}

int initialize() {
    return 100;  // Example initialization value
}

int main() {
    {
        atom::async::ThreadLocal<int> threadLocal;  // No initializer

        std::thread t1(threadFunction, std::ref(threadLocal));
        std::thread t2(threadFunction, std::ref(threadLocal));

        t1.join();
        t2.join();
    }

    {
        atom::async::ThreadLocal<int> threadLocal(
            initialize);  // With initializer

        std::thread t1(threadFunction, std::ref(threadLocal));
        std::thread t2(threadFunction, std::ref(threadLocal));

        t1.join();
        t2.join();
    }
    return 0;
}
