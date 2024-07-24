#ifndef LITHIUM_ADDON_TEMPLATE_STANDALONE_HPP
#define LITHIUM_ADDON_TEMPLATE_STANDALONE_HPP

#include <memory>
#include "atom/components/component.hpp"

class StandAloneComponentImpl;
class StandAloneComponent : public Component {
public:
    explicit StandAloneComponent(std::string name);
    ~StandAloneComponent() override;

    void startLocalDriver(const std::string& driver_name);

    void stopLocalDriver();

    void monitorDrivers();

    void processMessages();

    void sendMessageToDriver(std::string_view message);

    void printDriver();

    void toggleDriverListening();

private:
    auto createPipes(int stdinPipe[2], int stdoutPipe[2]) -> bool;

    void backgroundProcessing();

#if defined(_WIN32) || defined(_WIN64)
    void startWindowsProcess(const std::string& driver_name, int stdinPipe[2],
                             int stdoutPipe[2]);
#else
    void startUnixProcess(const std::string& driver_name, int stdinPipe[2],
                          int stdoutPipe[2]);

    auto createSharedMemory(int& shm_fd, int*& shm_ptr) -> bool;
    void closeSharedMemory(int shm_fd, int* shm_ptr);
    auto createSemaphore(sem_t*& sem) -> bool;
    void handleChildProcess(const std::string& driver_name, int stdinPipe[2],
                            int stdoutPipe[2], int* shm_ptr, sem_t* sem,
                            int shm_fd);
    void handleParentProcess(pid_t pid, int stdinPipe[2], int stdoutPipe[2],
                             int* shm_ptr, sem_t* sem, int shm_fd);
#endif
    std::unique_ptr<StandAloneComponentImpl> impl_;
};

#endif