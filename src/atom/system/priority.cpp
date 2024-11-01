#include "priority.hpp"

#include <cstring>

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"
#include "atom/utils/to_string.hpp"

namespace atom::system {
void PriorityManager::setProcessPriority(PriorityLevel level, int pid) {
    LOG_F(INFO, "Setting process priority to {} for PID {}",
          static_cast<int>(level), pid);
#ifdef _WIN32
    DWORD priority = getPriorityFromLevel(level);
    HANDLE hProcess = pid == 0
                          ? GetCurrentProcess()
                          : OpenProcess(PROCESS_SET_INFORMATION, FALSE, pid);
    if (hProcess == nullptr) {
        LOG_F(ERROR, "Failed to open process: {}", GetLastError());
        THROW_RUNTIME_ERROR("Failed to open process: " +
                            std::to_string(GetLastError()));
    }
    if (SetPriorityClass(hProcess, priority) == 0) {
        CloseHandle(hProcess);
        LOG_F(ERROR, "Failed to set process priority: {}", GetLastError());
        THROW_RUNTIME_ERROR("Failed to set process priority: " +
                            std::to_string(GetLastError()));
    }
    if (pid != 0) {
        CloseHandle(hProcess);
    }
#else
    int priority = getPriorityFromLevel(level);
    if (setpriority(PRIO_PROCESS, pid, priority) == -1) {
        LOG_F(ERROR, "Failed to set process priority: {}", strerror(errno));
        THROW_RUNTIME_ERROR("Failed to set process priority: " +
                            std::string(strerror(errno)));
    }
#endif
    LOG_F(INFO, "Set process priority to {} for PID {}",
          static_cast<int>(level), pid);
}

auto PriorityManager::getProcessPriority(int pid) -> PriorityLevel {
    LOG_F(INFO, "Getting process priority for PID {}", pid);
#ifdef _WIN32
    HANDLE hProcess = pid == 0
                          ? GetCurrentProcess()
                          : OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (hProcess == nullptr) {
        LOG_F(ERROR, "Failed to open process: {}", GetLastError());
        THROW_RUNTIME_ERROR("Failed to open process: " +
                            std::to_string(GetLastError()));
    }
    DWORD priority = GetPriorityClass(hProcess);
    if (pid != 0) {
        CloseHandle(hProcess);
    }
    if (priority == 0) {
        LOG_F(ERROR, "Failed to get process priority: {}", GetLastError());
        THROW_RUNTIME_ERROR("Failed to get process priority: " +
                            std::to_string(GetLastError()));
    }
    PriorityLevel level = getLevelFromPriority(priority);
    LOG_F(INFO, "Got process priority {} for PID {}", static_cast<int>(level),
          pid);
    return level;
#else
    int priority = getpriority(PRIO_PROCESS, pid);
    if (priority == -1 && errno != 0) {
        LOG_F(ERROR, "Failed to get process priority: {}", strerror(errno));
        THROW_RUNTIME_ERROR("Failed to get process priority: " +
                            std::string(strerror(errno)));
    }
    PriorityLevel level = getLevelFromPriority(priority);
    LOG_F(INFO, "Got process priority {} for PID {}", static_cast<int>(level),
          pid);
    return level;
#endif
}

void PriorityManager::setThreadPriority(
    PriorityLevel level, std::thread::native_handle_type thread) {
    LOG_F(INFO, "Setting thread priority to {}", static_cast<int>(level));
#ifdef _WIN32
    HANDLE hThread =
        thread == 0 ? GetCurrentThread() : reinterpret_cast<HANDLE>(thread);
    if (SetThreadPriority(hThread, getThreadPriorityFromLevel(level)) == 0) {
        LOG_F(ERROR, "Failed to set thread priority: {}", GetLastError());
        THROW_RUNTIME_ERROR("Failed to set thread priority: " +
                            std::to_string(GetLastError()));
    }
#else
    int policy;
    struct sched_param param;
    pthread_t threadId = thread == 0 ? pthread_self() : thread;
    pthread_getschedparam(threadId, &policy, &param);
    param.sched_priority = getThreadPriorityFromLevel(level);
    if (pthread_setschedparam(threadId, SCHED_RR, &param) != 0) {
        LOG_F(ERROR, "Failed to set thread priority: {}", strerror(errno));
        THROW_RUNTIME_ERROR("Failed to set thread priority: " +
                            std::string(strerror(errno)));
    }
#endif
    LOG_F(INFO, "Set thread priority to {}", static_cast<int>(level));
}

auto PriorityManager::getThreadPriority(std::thread::native_handle_type thread)
    -> PriorityLevel {
    LOG_F(INFO, "Getting thread priority");
#ifdef _WIN32
    HANDLE hThread =
        thread == 0 ? GetCurrentThread() : reinterpret_cast<HANDLE>(thread);
    int priority = GetThreadPriority(hThread);
    if (priority == THREAD_PRIORITY_ERROR_RETURN) {
        LOG_F(ERROR, "Failed to get thread priority: {}", GetLastError());
        THROW_RUNTIME_ERROR("Failed to get thread priority: " +
                            std::to_string(GetLastError()));
    }
    PriorityLevel level = getLevelFromThreadPriority(priority);
    LOG_F(INFO, "Got thread priority {}", static_cast<int>(level));
    return level;
#else
    int policy;
    struct sched_param param;
    pthread_t threadId = thread == 0 ? pthread_self() : thread;
    if (pthread_getschedparam(threadId, &policy, &param) != 0) {
        LOG_F(ERROR, "Failed to get thread priority: {}", strerror(errno));
        THROW_RUNTIME_ERROR("Failed to get thread priority: " +
                            std::string(strerror(errno)));
    }
    PriorityLevel level = getLevelFromThreadPriority(param.sched_priority);
    LOG_F(INFO, "Got thread priority {}", static_cast<int>(level));
    return level;
#endif
}

void PriorityManager::setThreadSchedulingPolicy(
    SchedulingPolicy policy, std::thread::native_handle_type thread) {
    LOG_F(INFO, "Setting thread scheduling policy to {}",
          static_cast<int>(policy));
#ifdef _WIN32
    LOG_F(ERROR,
          "Changing thread scheduling policy is not supported on Windows");
    THROW_RUNTIME_ERROR(
        "Changing thread scheduling policy is not supported on Windows");
#else
    int nativePolicy;
    struct sched_param param;
    pthread_t threadId = thread == 0 ? pthread_self() : thread;

    switch (policy) {
        case SchedulingPolicy::NORMAL:
            nativePolicy = SCHED_OTHER;
            break;
        case SchedulingPolicy::FIFO:
            nativePolicy = SCHED_FIFO;
            break;
        case SchedulingPolicy::ROUND_ROBIN:
            nativePolicy = SCHED_RR;
            break;
        default:
            LOG_F(ERROR, "Invalid scheduling policy: {}",
                  static_cast<int>(policy));
            THROW_INVALID_ARGUMENT("Invalid scheduling policy");
    }

    pthread_getschedparam(threadId, &nativePolicy, &param);
    if (pthread_setschedparam(threadId, nativePolicy, &param) != 0) {
        LOG_F(ERROR, "Failed to set thread scheduling policy: {}",
              strerror(errno));
        THROW_RUNTIME_ERROR("Failed to set thread scheduling policy: " +
                            std::string(strerror(errno)));
    }
#endif
    LOG_F(INFO, "Set thread scheduling policy to {}", static_cast<int>(policy));
}

void PriorityManager::setProcessAffinity(const std::vector<int>& cpus,
                                         int pid) {
    LOG_F(INFO, "Setting process affinity to CPUs: {} for PID {}",
          atom::utils::toString(cpus), pid);
#ifdef _WIN32
    HANDLE hProcess = pid == 0
                          ? GetCurrentProcess()
                          : OpenProcess(PROCESS_SET_INFORMATION, FALSE, pid);
    if (hProcess == nullptr) {
        LOG_F(ERROR, "Failed to open process: {}", GetLastError());
        THROW_RUNTIME_ERROR("Failed to open process: " +
                            std::to_string(GetLastError()));
    }
    DWORD_PTR mask = 0;
    for (int cpu : cpus) {
        mask |= (1ULL << cpu);
    }
    if (SetProcessAffinityMask(hProcess, mask) == 0) {
        CloseHandle(hProcess);
        LOG_F(ERROR, "Failed to set process affinity: {}", GetLastError());
        THROW_RUNTIME_ERROR("Failed to set process affinity: " +
                            std::to_string(GetLastError()));
    }
    if (pid != 0) {
        CloseHandle(hProcess);
    }
#else
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    for (int cpu : cpus) {
        CPU_SET(cpu, &cpuset);
    }
    if (sched_setaffinity(pid, sizeof(cpu_set_t), &cpuset) == -1) {
        LOG_F(ERROR, "Failed to set process affinity: {}", strerror(errno));
        THROW_RUNTIME_ERROR("Failed to set process affinity: " +
                            std::string(strerror(errno)));
    }
#endif
    LOG_F(INFO, "Set process affinity to CPUs: {} for PID {}",
          atom::utils::toString(cpus), pid);
}

auto PriorityManager::getProcessAffinity(int pid) -> std::vector<int> {
    LOG_F(INFO, "Getting process affinity for PID {}", pid);
    std::vector<int> cpus;
#ifdef _WIN32
    HANDLE hProcess = pid == 0
                          ? GetCurrentProcess()
                          : OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (hProcess == nullptr) {
        LOG_F(ERROR, "Failed to open process: {}", GetLastError());
        THROW_RUNTIME_ERROR("Failed to open process: " +
                            std::to_string(GetLastError()));
    }
    DWORD_PTR processAffinityMask;
    DWORD_PTR systemAffinityMask;
    if (GetProcessAffinityMask(hProcess, &processAffinityMask,
                               &systemAffinityMask) == 0) {
        CloseHandle(hProcess);
        LOG_F(ERROR, "Failed to get process affinity: {}", GetLastError());
        THROW_RUNTIME_ERROR("Failed to get process affinity: " +
                            std::to_string(GetLastError()));
    }
    if (pid != 0) {
        CloseHandle(hProcess);
    }
    for (int i = 0; i < 64; i++) {
        if ((processAffinityMask & (1ULL << i)) != 0u) {
            cpus.push_back(i);
        }
    }
#else
    cpu_set_t cpuset;
    if (sched_getaffinity(pid, sizeof(cpu_set_t), &cpuset) == -1) {
        LOG_F(ERROR, "Failed to get process affinity: {}", strerror(errno));
        THROW_RUNTIME_ERROR("Failed to get process affinity: " +
                            std::string(strerror(errno)));
    }
    for (int i = 0; i < CPU_SETSIZE; i++) {
        if (CPU_ISSET(i, &cpuset)) {
            cpus.push_back(i);
        }
    }
#endif
    LOG_F(INFO, "Got process affinity for PID {}: {}", pid,
          atom::utils::toString(cpus));
    return cpus;
}

void PriorityManager::startPriorityMonitor(
    int pid, const std::function<void(PriorityLevel)>& callback,
    std::chrono::milliseconds interval) {
    LOG_F(INFO, "Starting priority monitor for PID {}", pid);
    std::thread([pid, callback, interval]() {
        PriorityLevel lastPriority = getProcessPriority(pid);
        while (true) {
            std::this_thread::sleep_for(interval);
            try {
                PriorityLevel currentPriority = getProcessPriority(pid);
                if (currentPriority != lastPriority) {
                    callback(currentPriority);
                    lastPriority = currentPriority;
                }
            } catch (const std::exception& e) {
                LOG_F(ERROR, "Error in priority monitor: {}", e.what());
                break;
            }
        }
    }).detach();
    LOG_F(INFO, "Started priority monitor for PID {}", pid);
}

#ifdef _WIN32

auto PriorityManager::getPriorityFromLevel(PriorityLevel level) -> DWORD {
    LOG_F(INFO, "Getting priority from level {}", static_cast<int>(level));
    switch (level) {
        case PriorityLevel::LOWEST:
            return IDLE_PRIORITY_CLASS;
        case PriorityLevel::BELOW_NORMAL:
            return BELOW_NORMAL_PRIORITY_CLASS;
        case PriorityLevel::NORMAL:
            return NORMAL_PRIORITY_CLASS;
        case PriorityLevel::ABOVE_NORMAL:
            return ABOVE_NORMAL_PRIORITY_CLASS;
        case PriorityLevel::HIGHEST:
            return HIGH_PRIORITY_CLASS;
        case PriorityLevel::REALTIME:
            return REALTIME_PRIORITY_CLASS;
        default:
            LOG_F(ERROR, "Invalid priority level: {}", static_cast<int>(level));
            throw std::invalid_argument("Invalid priority level");
    }
}

auto PriorityManager::getLevelFromPriority(DWORD priority) -> PriorityLevel {
    LOG_F(INFO, "Getting level from priority {}", priority);
    switch (priority) {
        case IDLE_PRIORITY_CLASS:
            return PriorityLevel::LOWEST;
        case BELOW_NORMAL_PRIORITY_CLASS:
            return PriorityLevel::BELOW_NORMAL;
        case NORMAL_PRIORITY_CLASS:
            return PriorityLevel::NORMAL;
        case ABOVE_NORMAL_PRIORITY_CLASS:
            return PriorityLevel::ABOVE_NORMAL;
        case HIGH_PRIORITY_CLASS:
            return PriorityLevel::HIGHEST;
        case REALTIME_PRIORITY_CLASS:
            return PriorityLevel::REALTIME;
        default:
            LOG_F(ERROR, "Invalid priority value: {}", priority);
            throw std::invalid_argument("Invalid priority value");
    }
}

auto PriorityManager::getThreadPriorityFromLevel(PriorityLevel level) -> int {
    LOG_F(INFO, "Getting thread priority from level {}",
          static_cast<int>(level));
    switch (level) {
        case PriorityLevel::LOWEST:
            return THREAD_PRIORITY_IDLE;
        case PriorityLevel::BELOW_NORMAL:
            return THREAD_PRIORITY_BELOW_NORMAL;
        case PriorityLevel::NORMAL:
            return THREAD_PRIORITY_NORMAL;
        case PriorityLevel::ABOVE_NORMAL:
            return THREAD_PRIORITY_ABOVE_NORMAL;
        case PriorityLevel::HIGHEST:
            return THREAD_PRIORITY_HIGHEST;
        case PriorityLevel::REALTIME:
            return THREAD_PRIORITY_TIME_CRITICAL;
        default:
            LOG_F(ERROR, "Invalid priority level: {}", static_cast<int>(level));
            throw std::invalid_argument("Invalid priority level");
    }
}

auto PriorityManager::getLevelFromThreadPriority(int priority)
    -> PriorityLevel {
    LOG_F(INFO, "Getting level from thread priority {}", priority);
    switch (priority) {
        case THREAD_PRIORITY_IDLE:
            return PriorityLevel::LOWEST;
        case THREAD_PRIORITY_BELOW_NORMAL:
            return PriorityLevel::BELOW_NORMAL;
        case THREAD_PRIORITY_NORMAL:
            return PriorityLevel::NORMAL;
        case THREAD_PRIORITY_ABOVE_NORMAL:
            return PriorityLevel::ABOVE_NORMAL;
        case THREAD_PRIORITY_HIGHEST:
            return PriorityLevel::HIGHEST;
        case THREAD_PRIORITY_TIME_CRITICAL:
            return PriorityLevel::REALTIME;
        default:
            LOG_F(ERROR, "Unknown thread priority value: {}", priority);
            throw std::runtime_error("Unknown thread priority value");
    }
}

#else

auto PriorityManager::getPriorityFromLevel(PriorityLevel level) -> int {
    LOG_F(INFO, "Getting priority from level {}", static_cast<int>(level));
    switch (level) {
        case PriorityLevel::LOWEST:
            return 19;  // Lowest nice value
        case PriorityLevel::BELOW_NORMAL:
            return 10;
        case PriorityLevel::NORMAL:
            return 0;
        case PriorityLevel::ABOVE_NORMAL:
            return -10;
        case PriorityLevel::HIGHEST:
            return -20;  // Highest nice value
        case PriorityLevel::REALTIME:
            return sched_get_priority_max(SCHED_FIFO);  // Real-time priority
        default:
            LOG_F(ERROR, "Invalid priority level: {}", static_cast<int>(level));
            throw std::invalid_argument("Invalid priority level");
    }
}

auto PriorityManager::getLevelFromPriority(int priority) -> PriorityLevel {
    LOG_F(INFO, "Getting level from priority {}", priority);
    if (priority == 19) {
        return PriorityLevel::LOWEST;
    } else if (priority == 10) {
        return PriorityLevel::BELOW_NORMAL;
    } else if (priority == 0) {
        return PriorityLevel::NORMAL;
    } else if (priority == -10) {
        return PriorityLevel::ABOVE_NORMAL;
    } else if (priority == -20) {
        return PriorityLevel::HIGHEST;
    } else if (priority == sched_get_priority_max(SCHED_FIFO)) {
        return PriorityLevel::REALTIME;
    } else {
        LOG_F(ERROR, "Invalid priority value: {}", priority);
        throw std::invalid_argument("Invalid priority value");
    }
}

auto PriorityManager::getThreadPriorityFromLevel(PriorityLevel level) -> int {
    LOG_F(INFO, "Getting thread priority from level {}",
          static_cast<int>(level));
    switch (level) {
        case PriorityLevel::LOWEST:
            return 19;  // Lowest nice value
        case PriorityLevel::BELOW_NORMAL:
            return 10;
        case PriorityLevel::NORMAL:
            return 0;
        case PriorityLevel::ABOVE_NORMAL:
            return -10;
        case PriorityLevel::HIGHEST:
            return -20;  // Highest nice value
        case PriorityLevel::REALTIME:
            return sched_get_priority_max(SCHED_FIFO);  // Real-time priority
        default:
            LOG_F(ERROR, "Invalid priority level: {}", static_cast<int>(level));
            throw std::invalid_argument("Invalid priority level");
    }
}

auto PriorityManager::getLevelFromThreadPriority(int priority)
    -> PriorityLevel {
    LOG_F(INFO, "Getting level from thread priority {}", priority);
    if (priority == 19) {
        return PriorityLevel::LOWEST;
    } else if (priority == 10) {
        return PriorityLevel::BELOW_NORMAL;
    } else if (priority == 0) {
        return PriorityLevel::NORMAL;
    } else if (priority == -10) {
        return PriorityLevel::ABOVE_NORMAL;
    } else if (priority == -20) {
        return PriorityLevel::HIGHEST;
    } else if (priority == sched_get_priority_max(SCHED_FIFO)) {
        return PriorityLevel::REALTIME;
    } else {
        LOG_F(ERROR, "Unknown thread priority value: {}", priority);
        throw std::runtime_error("Unknown thread priority value");
    }
}

#endif
}  // namespace atom::system
