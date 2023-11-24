#pragma once

#include <string>

#ifdef USE_LIBUV

#include <string>
#include <uv.h>

class Fifo
{
    std::string name; /* Path to FIFO for dynamic startups & shutdowns of drivers */

    char buffer[1024];
    int bufferPos = 0;
    int fd = -1;
    uv_poll_t pollHandle;

    void close();
    void open();
    void processLine(const char *line);

    /* Read commands from FIFO and process them. Start/stop drivers accordingly */
    void read();
    static void ioCb(uv_poll_t* handle, int status, int revents);

public:
    Fifo(const std::string& name);
    void listen()
    {
        open();
    }
};

#else

#include <ev++.h>

class Fifo
{
public:
    Fifo(const std::string &name);

    void listen()
    {
        open();
    }

    void processLine(const char *line);

private:
    void close();
    void open();
    /* Read commands from FIFO and process them. Start/stop drivers accordingly */
    void read();
    void ioCb(ev::io &watcher, int revents);

    std::string name; /* Path to FIFO for dynamic startups & shutdowns of drivers */
    char buffer[1024];
    int bufferPos = 0;
    int fd = -1;
    ev::io fdev;
};
#endif