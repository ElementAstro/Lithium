#include <chrono>
#include <iostream>
#include <thread>

#include "atom/async/daemon.hpp"

int mainCallback(int argc, char **argv) {
    std::cout << "Daemon process running...\n";

    // Simulate some work in the daemon
    for (int i = 0; i < 10; ++i) {
        std::cout << "Daemon is working: " << i + 1 << "/10" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;  // Indicate success
}

int main(int argc, char **argv) {
    atom::async::DaemonGuard daemonGuard;

    // Set up signal handling
    signal(SIGTERM, atom::async::signalHandler);
    signal(SIGINT, atom::async::signalHandler);

    // Start the daemon
    daemonGuard.startDaemon(argc, argv, mainCallback, true);

    return 0;
}
