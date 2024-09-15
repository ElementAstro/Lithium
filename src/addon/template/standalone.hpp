#ifndef LITHIUM_ADDON_TEMPLATE_STANDALONE_HPP
#define LITHIUM_ADDON_TEMPLATE_STANDALONE_HPP

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <variant>

#include "atom/components/component.hpp"

enum class InteractionMethod { Pipe, FIFO, SharedMemory };

class StandAloneComponentImpl;

class StandAloneComponent : public Component {
public:
    explicit StandAloneComponent(std::string name);
    ~StandAloneComponent() override;

    void startLocalDriver(const std::string& driver_name,
                          InteractionMethod method);
    void stopLocalDriver();
    void monitorDrivers();
    void processMessages();
    void sendMessageToDriver(std::string_view message);
    void printDriver() const;
    void toggleDriverListening();

private:
    auto createPipes() -> std::optional<std::pair<int, int>>;
    auto createFIFO() -> std::optional<std::pair<int, int>>;
    auto createSharedMemory() -> std::optional<std::pair<int, int*>>;

    void backgroundProcessing();

#if defined(_WIN32) || defined(_WIN64)
    void startWindowsProcess(
        const std::string& driver_name,
        std::variant<std::pair<int, int>, std::pair<int, int*>> io);
#else
    void startUnixProcess(
        const std::string& driver_name,
        std::variant<std::pair<int, int>, std::pair<int, int*>> io);
    void handleChildProcess(
        const std::string& driver_name,
        std::variant<std::pair<int, int>, std::pair<int, int*>> io,
        int* shm_ptr, sem_t* sem, int shm_fd);
    void handleParentProcess(
        pid_t pid, std::variant<std::pair<int, int>, std::pair<int, int*>> io,
        int* shm_ptr, sem_t* sem, int shm_fd);
#endif

    auto createSemaphore() -> std::optional<sem_t*>;
    void closeSharedMemory(int shm_fd, int* shm_ptr);

    std::unique_ptr<StandAloneComponentImpl> impl_;
};

#endif
