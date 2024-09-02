#include "atom/connection/sshserver.hpp"

#include <chrono>
#include <iostream>
#include <thread>

// Function to run the SSH server
void runSshServer(const std::filesystem::path& configFile) {
    atom::connection::SshServer sshServer(configFile);

    // Configure the SSH server
    sshServer.setPort(22);                  // Set port for SSH
    sshServer.setListenAddress("0.0.0.0");  // Listen on all interfaces
    sshServer.setHostKey("/etc/ssh/ssh_host_rsa_key");  // Set the host key file

    // Allow password authentication
    sshServer.setPasswordAuthentication(true);

    // Allow root login (not recommended in production)
    sshServer.allowRootLogin(true);

    // Start the SSH server
    sshServer.start();
    std::cout << "SSH Server started on port " << sshServer.getPort()
              << std::endl;

    // Keep the server running for a while
    std::this_thread::sleep_for(std::chrono::seconds(60));

    // Stop the SSH server
    sshServer.stop();
    std::cout << "SSH Server stopped." << std::endl;
}

int main() {
    const std::filesystem::path configFile =
        "/path/to/your/sshconfig.file";  // Update this path to your
                                         // configuration file

    // Start the SSH server in a separate thread
    std::thread serverThread(runSshServer, configFile);

    // Wait for the server thread to finish
    serverThread.join();

    return 0;
}
