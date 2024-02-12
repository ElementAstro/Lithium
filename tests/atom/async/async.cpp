#include "atom/async/async.hpp"

using namespace Atom::Async;
int main()
{
    AsyncWorkerManager<int> manager;

    auto worker1 = manager.CreateWorker([]()
                                        {
        std::this_thread::sleep_for(std::chrono::seconds(3));
        return 42; });

    auto worker2 = manager.CreateWorker([]()
                                        {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        return 100; });

    manager.WaitForAll();

    std::cout << "All workers are done." << std::endl;

    if (manager.IsDone(worker1))
    {
        std::cout << "Worker 1 is done." << std::endl;
    }

    manager.Cancel(worker2);
    std::cout << "Worker 2 is cancelled." << std::endl;

    return 0;
}