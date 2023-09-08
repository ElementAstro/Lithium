#pragma once

#include <string>

#include <ev++.h>

class Fifo
{
    std::string name; /* Path to FIFO for dynamic startups & shutdowns of drivers */

    char buffer[1024];
    int bufferPos = 0;
    int fd = -1;
    ev::io fdev;

    void close();
    void open();
    void processLine(const char *line);

    /* Read commands from FIFO and process them. Start/stop drivers accordingly */
    void read();
    void ioCb(ev::io &watcher, int revents);

public:
    Fifo(const std::string &name);
    void listen()
    {
        open();
    }
};