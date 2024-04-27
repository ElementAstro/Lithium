#include <iostream>
#include <string>
#include <thread>

using namespace Atom::Async;

void producer(ThreadSafeQueue<int>& queue) {
    for (int i = 0; i < 10; ++i) {
        queue.put(i);
        std::cout << "Produced: " << i << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

void consumer(ThreadSafeQueue<int>& queue) {
    while (true) {
        auto item = queue.take();
        if (!item) {
            break;
        }
        std::cout << "Consumed: " << *item << std::endl;
    }
}

int main() {
    ThreadSafeQueue<int> queue;

    std::thread t1(producer, std::ref(queue));
    std::thread t2(consumer, std::ref(queue));

    t1.join();
    queue.destroy();
    t2.join();

    // 使用 emplace
    ThreadSafeQueue<std::string> strQueue;
    strQueue.emplace("Hello");
    strQueue.emplace("World");
    std::cout << "Front: " << *strQueue.front() << std::endl;
    std::cout << "Back: " << *strQueue.back() << std::endl;

    // 使用 waitFor
    ThreadSafeQueue<int> intQueue;
    std::thread t3([&intQueue]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        intQueue.put(42);
    });
    auto item =
        intQueue.waitFor([](const std::queue<int>& q) { return !q.empty(); });
    std::cout << "Waited for: " << *item << std::endl;
    t3.join();

    // 使用 extractIf
    /*
    ThreadSafeQueue<int> extractQueue;
    for (int i = 0; i < 10; ++i) {
        extractQueue.put(i);
    }
    auto evenNumbers = extractQueue.extractIf([](int i) { return i % 2 == 0; });
    std::cout << "Extracted even numbers: ";
    for (auto num : evenNumbers) {
        std::cout << num << " ";
    }
    std::cout << std::endl;
    */
    

    // 使用 sort
    ThreadSafeQueue<int> sortQueue;
    sortQueue.put(3);
    sortQueue.put(1);
    sortQueue.put(4);
    sortQueue.put(1);
    sortQueue.put(5);
    sortQueue.sort(std::greater<int>());
    std::cout << "Sorted queue: ";
    while (!sortQueue.empty()) {
        std::cout << *sortQueue.take() << " ";
    }
    std::cout << std::endl;

    // 使用 transform
    ThreadSafeQueue<int> transformQueue;
    for (int i = 1; i <= 5; ++i) {
        transformQueue.put(i);
    }
    /*auto squaredQueue =
        transformQueue.transform<int>([](int i) { return i * i; });
    std::cout << "Squared queue: ";
    while (!squaredQueue.empty()) {
        std::cout << *squaredQueue.take() << " ";
    }
    std::cout << std::endl;*/

    // 使用 groupBy
    /*
    ThreadSafeQueue<std::string> groupByQueue;
    groupByQueue.put("apple");
    groupByQueue.put("banana");
    groupByQueue.put("cherry");
    groupByQueue.put("date");
    groupByQueue.put("elderberry");
    auto groupedQueues = groupByQueue.groupBy<std::string>(
        [](const std::string& s) -> std::string {
            return std::to_string(s[0]);
        });
    std::cout << "Grouped queues: " << std::endl;
    for (auto& queue : groupedQueues) {
        std::cout << "Group: ";
        while (!queue.empty()) {
            std::cout << *queue.take() << " ";
        }
        std::cout << std::endl;
    }
    */

    return 0;
}