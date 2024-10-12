#ifndef SIGNAL_HANDLER_H
#define SIGNAL_HANDLER_H

#include <atomic>
#include <functional>
#include <map>
#include <mutex>
#include <set>
#include <thread>
#include <vector>

#if defined(_WIN32) || defined(_WIN64)
#include <csignal>
#define NOMINMAX
#include <windows.h>
#else
#include <signal.h>
#endif

/**
 * @brief Type alias for signal identifiers.
 */
using SignalID = int;

/**
 * @brief Type alias for signal handler functions.
 */
using SignalHandler = std::function<void(SignalID)>;

/**
 * @brief Structure to associate a signal handler with a priority.
 *
 * Handlers with higher priority values will be executed first.
 */
struct SignalHandlerWithPriority {
    SignalHandler handler;  ///< The signal handler function.
    int priority;           ///< The priority of the handler.

    /**
     * @brief Compare two `SignalHandlerWithPriority` objects based on priority.
     *
     * @param other The other `SignalHandlerWithPriority` object to compare
     * against.
     * @return `true` if this object's priority is greater than the other's;
     * `false` otherwise.
     */
    auto operator<(const SignalHandlerWithPriority& other) const -> bool;
};

/**
 * @brief Singleton class to manage signal handlers and dispatch signals.
 *
 * This class handles registering and dispatching signal handlers with
 * priorities. It also provides a mechanism to set up default crash signal
 * handlers.
 */
class SignalHandlerRegistry {
public:
    /**
     * @brief Get the singleton instance of the `SignalHandlerRegistry`.
     *
     * @return A reference to the singleton `SignalHandlerRegistry` instance.
     */
    static auto getInstance() -> SignalHandlerRegistry&;

    /**
     * @brief Set a signal handler for a specific signal with an optional
     * priority.
     *
     * @param signal The signal ID to handle.
     * @param handler The handler function to execute.
     * @param priority The priority of the handler. Default is 0.
     */
    void setSignalHandler(SignalID signal, const SignalHandler& handler,
                          int priority = 0);

    /**
     * @brief Remove a specific signal handler for a signal.
     *
     * @param signal The signal ID to stop handling.
     * @param handler The handler function to remove.
     */
    void removeSignalHandler(SignalID signal, const SignalHandler& handler);

    /**
     * @brief Set handlers for standard crash signals.
     *
     * @param handler The handler function to execute for crash signals.
     * @param priority The priority of the handler. Default is 0.
     */
    void setStandardCrashHandlerSignals(const SignalHandler& handler,
                                        int priority = 0);

private:
    SignalHandlerRegistry();
    ~SignalHandlerRegistry();

    static void signalDispatcher(int signal);

    static auto getStandardCrashSignals() -> std::set<SignalID>;

    std::map<SignalID, std::set<SignalHandlerWithPriority>>
        handlers_;      ///< Map of signal IDs to handlers with priorities.
    std::mutex mutex_;  ///< Mutex for synchronizing access to the handlers.
};

#endif  // SIGNAL_HANDLER_H

#ifndef SAFE_SIGNAL_MANAGER_H
#define SAFE_SIGNAL_MANAGER_H

#include <atomic>
#include <vector>

/**
 * @brief Class to safely manage and dispatch signals with separate thread
 * handling.
 *
 * This class allows adding and removing signal handlers and dispatching signals
 * in a separate thread to ensure thread safety and avoid blocking signal
 * handling.
 */
class SafeSignalManager {
public:
    /**
     * @brief Constructs a `SafeSignalManager` and starts the signal processing
     * thread.
     */
    SafeSignalManager();

    /**
     * @brief Destructs the `SafeSignalManager` and stops the signal processing
     * thread.
     */
    ~SafeSignalManager();

    /**
     * @brief Add a signal handler for a specific signal with an optional
     * priority.
     *
     * @param signal The signal ID to handle.
     * @param handler The handler function to execute.
     * @param priority The priority of the handler. Default is 0.
     */
    void addSafeSignalHandler(SignalID signal, const SignalHandler& handler,
                              int priority = 0);

    /**
     * @brief Remove a specific signal handler for a signal.
     *
     * @param signal The signal ID to stop handling.
     * @param handler The handler function to remove.
     */
    void removeSafeSignalHandler(SignalID signal, const SignalHandler& handler);

    void clearSignalQueue();

    /**
     * @brief Static method to safely dispatch signals to the manager.
     *
     * This method is called by the system signal handler to queue signals for
     * processing.
     *
     * @param signal The signal ID to queue.
     */
    static void safeSignalDispatcher(int signal);

    /**
     * @brief Get the singleton instance of `SafeSignalManager`.
     *
     * @return A reference to the singleton `SafeSignalManager` instance.
     */
    static auto getInstance() -> SafeSignalManager&;

private:
    /**
     * @brief Process signals from the queue in a separate thread.
     */
    void processSignals();

    std::atomic<bool> keepRunning_;  ///< Flag to control the running state of
                                     ///< the signal processing thread.
    std::map<SignalID, std::set<SignalHandlerWithPriority>>
        safeHandlers_;  ///< Map of signal IDs to handlers with priorities.
    std::vector<int> signalQueue_;  ///< Queue of signals to be processed.
    std::mutex
        queueMutex_;  ///< Mutex for synchronizing access to the signal queue.
    std::jthread signalHandlingThread_;  ///< Thread for processing signals.
};

#endif  // SAFE_SIGNAL_MANAGER_H
