#include "wsserver.hpp"

using namespace Lithium;

int main()
{
    WebSocketServer server(10);
    server.run(8080);
    return 0;
}