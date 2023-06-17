#include "server.hpp"

int main(int argc, char* argv[]) {
    int port = 12345;
    int max_connections = 10;

    if (argc >= 2) {
        port = atoi(argv[1]);
    }

    if (argc >= 3) {
        max_connections = atoi(argv[2]);
    }

    auto server = std::make_shared<SocketServer>(max_connections);
    server->run(port);

    return 0;
}
