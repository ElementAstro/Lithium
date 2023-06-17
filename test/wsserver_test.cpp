#include "wsserver.hpp"

using namespace OpenAPT;

int main()
{
    WebSocketServer server(10);
    server.run(8080);
    return 0;
}