#include "wsserver.hpp"

using namespace OpenAPT;

int main()
{
    WebSocketServer server;
    server.run(8080);
    return 0;
}