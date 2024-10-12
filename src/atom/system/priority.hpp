#ifndef PRIORITY_MANAGER_H
#define PRIORITY_MANAGER_H

#include <chrono>
#include <functional>
#include <string>
#include <thread>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#include <sched.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <unistd.h>
#endif

/**
 * @class PriorityManager
 * @brief Manages process and thread priorities and affinities.
 */
class PriorityManager {
public:
    /**
     * @enum PriorityLevel
     * @brief Defines various priority levels.
     */
    enum class PriorityLevel {
        LOWEST,        ///< Lowest priority level.
        BELOW_NORMAL,  ///< Below normal priority level.
        NORMAL,        ///< Normal priority level.
        ABOVE_NORMAL,  ///< Above normal priority level.
        HIGHEST,       ///< Highest priority level.
        REALTIME       ///< Realtime priority level.
    };

    /**
     * @enum SchedulingPolicy
     * @brief Defines various scheduling policies.
     */
    enum class SchedulingPolicy {
        NORMAL,      ///< Normal scheduling policy.
        FIFO,        ///< First In First Out scheduling policy.
        ROUND_ROBIN  ///< Round Robin scheduling policy.
    };

    /**
     * @brief Sets the priority of a process.
     * @param level The priority level to set.
     * @param pid The process ID. Defaults to 0, which means the current
     * process.
     */
    static void setProcessPriority(PriorityLevel level, int pid = 0);

    /**
     * @brief Gets the priority of a process.
     * @param pid The process ID. Defaults to 0, which means the current
     * process.
     * @return The current priority level of the process.
     */
    static auto getProcessPriority(int pid = 0) -> PriorityLevel;

    /**
     * @brief Sets the priority of a thread.
     * @param level The priority level to set.
     * @param thread The native handle of the thread. Defaults to 0, which means
     * the current thread.
     */
    static void setThreadPriority(PriorityLevel level,
                                  std::thread::native_handle_type thread = 0);

    /**
     * @brief Gets the priority of a thread.
     * @param thread The native handle of the thread. Defaults to 0, which means
     * the current thread.
     * @return The current priority level of the thread.
     */
    static auto getThreadPriority(std::thread::native_handle_type thread = 0)
        -> PriorityLevel;

    /**
     * @brief Sets the scheduling policy of a thread.
     * @param policy The scheduling policy to set.
     * @param thread The native handle of the thread. Defaults to 0, which means
     * the current thread.
     */
    static void setThreadSchedulingPolicy(
        SchedulingPolicy policy, std::thread::native_handle_type thread = 0);

    /**
     * @brief Sets the CPU affinity of a process.
     * @param cpus A vector of CPU indices to set the affinity to.
     * @param pid The process ID. Defaults to 0, which means the current
     * process.
     */
    static void setProcessAffinity(const std::vector<int>& cpus, int pid = 0);

    /**
     * @brief Gets the CPU affinity of a process.
     * @param pid The process ID. Defaults to 0, which means the current
     * process.
     * @return A vector of CPU indices the process is affinitized to.
     */
    static auto getProcessAffinity(int pid = 0) -> std::vector<int>;

    /**
     * @brief Starts monitoring the priority of a process.
     * @param pid The process ID to monitor.
     * @param callback The callback function to call when the priority changes.
     * @param interval The interval at which to check the priority. Defaults to
     * 1 second.
     */
    static void startPriorityMonitor(
        int pid, const std::function<void(PriorityLevel)>& callback,
        std::chrono::milliseconds interval = std::chrono::seconds(1));

private:
    /**
     * @brief Converts a vector of integers to a string.
     * @param vec The vector to convert.
     * @return The string representation of the vector.
     */
    static auto vectorToString(const std::vector<int>& vec) -> std::string;

    /**
     * @brief Converts a priority level to a platform-specific priority value.
     * @param level The priority level to convert.
     * @return The platform-specific priority value.
     */
    static auto getPriorityFromLevel(PriorityLevel level) -> DWORD;

    /**
     * @brief Converts a platform-specific priority value to a priority level.
     * @param priority The platform-specific priority value to convert.
     * @return The corresponding priority level.
     */
    static auto getLevelFromPriority(DWORD priority) -> PriorityLevel;

    /**
     * @brief Converts a priority level to a platform-specific thread priority
     * value.
     * @param level The priority level to convert.
     * @return The platform-specific thread priority value.
     */
    static auto getThreadPriorityFromLevel(PriorityLevel level) -> int;

    /**
     * @brief Converts a platform-specific thread priority value to a priority
     * level.
     * @param priority The platform-specific thread priority value to convert.
     * @return The corresponding priority level.
     */
    static auto getLevelFromThreadPriority(int priority) -> PriorityLevel;
};

#endif  // PRIORITY_MANAGER_H
