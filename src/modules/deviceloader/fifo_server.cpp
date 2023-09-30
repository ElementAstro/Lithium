#include "fifo_server.hpp"

#include <unistd.h>
#include <cstring>
#include <string>
#include <fcntl.h>

#include "io.hpp"
#include "driver_info.hpp"
#include "local_driver.hpp"
#include "remote_driver.hpp"
#include "hydrogen_server.hpp"

#ifdef USE_LIBUV

Fifo::Fifo(const std::string &name) : name(name)
{
    pollHandle.data = this;
}

/* Attempt to open up FIFO */
void Fifo::close()
{
    if (fd != -1)
    {
        ::close(fd);
        fd = -1;
        uv_poll_stop(&pollHandle);
    }
    bufferPos = 0;
}

void Fifo::open()
{
    /* Open up FIFO, if available */
#ifdef _WIN32
    fd = ::open(name.c_str(), O_RDONLY);
#else
    fd = ::open(name.c_str(), O_RDONLY | O_NONBLOCK | O_CLOEXEC);
#endif

    if (fd < 0)
    {
        // log(fmt("open(%s): %s.\n", name.c_str(), strerror(errno)));
        // Bye();
    }

    uv_poll_init(uv_default_loop(), &pollHandle, fd);
    uv_poll_start(&pollHandle, UV_READABLE, ioCb);
}

/* Handle one fifo command. Start/stop drivers accordingly */
void Fifo::processLine(const char *line)
{

    // log(fmt("FIFO: %s\n", line));

    char cmd[MAXSBUF], arg[4][1], var[4][MAXSBUF], tDriver[MAXSBUF], tName[MAXSBUF], envConfig[MAXSBUF],
        envSkel[MAXSBUF], envPrefix[MAXSBUF];

    memset(&tDriver[0], 0, sizeof(char) * MAXSBUF);
    memset(&tName[0], 0, sizeof(char) * MAXSBUF);
    memset(&envConfig[0], 0, sizeof(char) * MAXSBUF);
    memset(&envSkel[0], 0, sizeof(char) * MAXSBUF);
    memset(&envPrefix[0], 0, sizeof(char) * MAXSBUF);

    int n = 0;

    bool remoteDriver = !!strstr(line, "@");

    // If remote driver
    if (remoteDriver)
    {
        n = sscanf(line, "%s %511[^\n]", cmd, tDriver);

        // Remove quotes if any
        char *ptr = tDriver;
        int len = strlen(tDriver);
        while ((ptr = strstr(tDriver, "\"")))
        {
            memmove(ptr, ptr + 1, --len);
            ptr[len] = '\0';
        }
    }
    // If local driver
    else
    {
        n = sscanf(line, "%s %s -%1c \"%511[^\"]\" -%1c \"%511[^\"]\" -%1c \"%511[^\"]\" -%1c \"%511[^\"]\"", cmd,
                   tDriver, arg[0], var[0], arg[1], var[1], arg[2], var[2], arg[3], var[3]);
    }

    int n_args = (n - 2) / 2;

    int j = 0;
    for (j = 0; j < n_args; j++)
    {
        if (arg[j][0] == 'n')
        {
            strncpy(tName, var[j], MAXSBUF - 1);
            tName[MAXSBUF - 1] = '\0';
        }
        else if (arg[j][0] == 'c')
        {
            strncpy(envConfig, var[j], MAXSBUF - 1);
            envConfig[MAXSBUF - 1] = '\0';
        }
        else if (arg[j][0] == 's')
        {
            strncpy(envSkel, var[j], MAXSBUF - 1);
            envSkel[MAXSBUF - 1] = '\0';
        }
        else if (arg[j][0] == 'p')
        {
            strncpy(envPrefix, var[j], MAXSBUF - 1);
            envPrefix[MAXSBUF - 1] = '\0';
        }
    }

    bool startCmd;
    if (!strcmp(cmd, "start"))
        startCmd = 1;
    else
        startCmd = 0;

    if (startCmd)
    {

        DvrInfo *dp;
        if (remoteDriver == 0)
        {
            auto *localDp = new LocalDvrInfo();
            dp = localDp;
            // strncpy(dp->dev, tName, MAXSBUF);
            localDp->envDev = tName;
            localDp->envConfig = envConfig;
            localDp->envSkel = envSkel;
            localDp->envPrefix = envPrefix;
        }
        else
        {
            dp = new RemoteDvrInfo();
        }
        dp->name = tDriver;
        dp->start();
    }
    else
    {
        for (auto dp : DvrInfo::drivers)
        {
            if (dp == nullptr)
                continue;

            if (dp->name == tDriver)
            {
                /* If device name is given, check against it before shutting down */
                if (tName[0] && !dp->isHandlingDevice(tName))
                    continue;
                if (verbose)
                    // log(fmt("FIFO: Shutting down driver: %s\n", tDriver));

                    dp->restart = false;
                dp->close();
                break;
            }
        }
    }
}

void Fifo::read()
{
    int rd = ::read(fd, buffer + bufferPos, sizeof(buffer) - 1 - bufferPos);
    if (rd == 0)
    {
        if (bufferPos > 0)
        {
            buffer[bufferPos] = '\0';
            processLine(buffer);
        }
        close();
        open();
        return;
    }
    if (rd == -1)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return;

        // log(fmt("Fifo error: %s\n", strerror(errno)));
        close();
        open();
        return;
    }

    bufferPos += rd;

    for (int i = 0; i < bufferPos; ++i)
    {
        if (buffer[i] == '\n')
        {
            buffer[i] = 0;
            processLine(buffer);
            // shift the buffer
            i++;                                    /* count including nl */
            bufferPos -= i;                         /* remove from nexbuf */
            memmove(buffer, buffer + i, bufferPos); /* slide remaining to front */
            i = -1;                                 /* restart for loop scan */
        }
    }

    if ((unsigned)bufferPos >= sizeof(buffer) - 1)
    {
        // log(fmt("Fifo overflow"));
        close();
        open();
    }
}

void Fifo::ioCb(uv_poll_t *handle, int status, int revents)
{
    Fifo *fifo = static_cast<Fifo *>(handle->data);
    if (status < 0)
    {
        int sockErrno = readFdError(fifo->fd);
        if (sockErrno)
        {
            // log(fmt("Error on fifo: %s\n", strerror(sockErrno)));
            fifo->close();
            fifo->open();
        }
    }
    else if (revents & UV_READABLE)
    {
        fifo->read();
    }
}

#else

Fifo::Fifo(const std::string &name) : name(name)
{
    fdev.set<Fifo, &Fifo::ioCb>(this);
}

/* Attempt to open up FIFO */
void Fifo::close(void)
{
    if (fd != -1)
    {
        ::close(fd);
        fd = -1;
        fdev.stop();
    }
    bufferPos = 0;
}

void Fifo::open()
{
    /* Open up FIFO, if available */
#ifdef _WIN32
    fd = ::open(name.c_str(), O_RDONLY);
#else
    fd = ::open(name.c_str(), O_RDONLY | O_NONBLOCK | O_CLOEXEC);
#endif

    if (fd < 0)
    {
        // log(fmt("open(%s): %s.\n", name.c_str(), strerror(errno)));
        // Bye();
    }

    fdev.start(fd, EV_READ);
}

/* Handle one fifo command. Start/stop drivers accordingly */
void Fifo::processLine(const char *line)
{
    char cmd[MAXSBUF], arg[4][2], var[4][MAXSBUF], tDriver[MAXSBUF], tName[MAXSBUF],
        envConfig[MAXSBUF], envSkel[MAXSBUF], envPrefix[MAXSBUF];

    std::memset(&tDriver[0], 0, sizeof(char) * MAXSBUF);
    std::memset(&tName[0], 0, sizeof(char) * MAXSBUF);
    std::memset(&envConfig[0], 0, sizeof(char) * MAXSBUF);
    std::memset(&envSkel[0], 0, sizeof(char) * MAXSBUF);
    std::memset(&envPrefix[0], 0, sizeof(char) * MAXSBUF);

    int n = 0;

    bool remoteDriver = !!std::strstr(line, "@");

    // If remote driver
    if (remoteDriver)
    {
        n = std::sscanf(line, "%s %511[^\n]", cmd, tDriver);

        // Remove quotes if any
        char *ptr = tDriver;
        int len = std::strlen(tDriver);
        while ((ptr = std::strstr(tDriver, "\"")))
        {
            std::memmove(ptr, ptr + 1, --len);
            ptr[len] = '\0';
        }
    }
    // If local driver
    else
    {
        n = std::sscanf(line, "%s %s -%1c \"%511[^\"]\" -%1c \"%511[^\"]\" -%1c \"%511[^\"]\" -%1c \"%511[^\"]\"",
                        cmd, tDriver, arg[0], var[0], arg[1], var[1], arg[2], var[2], arg[3], var[3]);
    }

    int n_args = (n - 2) / 2;

    int j = 0;
    for (j = 0; j < n_args; j++)
    {
        if (arg[j][0] == 'n')
        {
            strncpy(tName, var[j], MAXSBUF - 1);
            tName[MAXSBUF - 1] = '\0';
        }
        else if (arg[j][0] == 'c')
        {
            strncpy(envConfig, var[j], MAXSBUF - 1);
            envConfig[MAXSBUF - 1] = '\0';
        }
        else if (arg[j][0] == 's')
        {
            strncpy(envSkel, var[j], MAXSBUF - 1);
            envSkel[MAXSBUF - 1] = '\0';
        }
        else if (arg[j][0] == 'p')
        {
            strncpy(envPrefix, var[j], MAXSBUF - 1);
            envPrefix[MAXSBUF - 1] = '\0';
        }
    }

    bool startCmd;
    if (!std::strcmp(cmd, "start"))
        startCmd = true;
    else
        startCmd = false;

    if (startCmd)
    {
        DvrInfo *dp;
        if (remoteDriver == false)
        {
            auto *localDp = new LocalDvrInfo();
            dp = localDp;
            localDp->envDev = tName;
            localDp->envConfig = envConfig;
            localDp->envSkel = envSkel;
            localDp->envPrefix = envPrefix;
        }
        else
        {
            dp = new RemoteDvrInfo();
        }
        dp->name = tDriver;
        dp->start();
    }
    else
    {
        for (auto dp : DvrInfo::drivers)
        {
            if (dp == nullptr)
                continue;

            if (dp->name == tDriver)
            {
                // If device name is given, check against it before shutting down
                if (tName[0] && !dp->isHandlingDevice(tName))
                    continue;

                dp->restart = false;
                dp->close();
                break;
            }
        }
    }
}

void Fifo::read(void)
{
    int rd = ::read(fd, buffer + bufferPos, sizeof(buffer) - 1 - bufferPos);
    if (rd == 0)
    {
        if (bufferPos > 0)
        {
            buffer[bufferPos] = '\0';
            processLine(buffer);
        }
        close();
        open();
        return;
    }
    if (rd == -1)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return;

        // log(fmt("Fifo error: %s\n", strerror(errno)));
        close();
        open();
        return;
    }

    bufferPos += rd;

    for (int i = 0; i < bufferPos; ++i)
    {
        if (buffer[i] == '\n')
        {
            buffer[i] = 0;
            processLine(buffer);
            // shift the buffer
            i++;                                    /* count including nl */
            bufferPos -= i;                         /* remove from nexbuf */
            memmove(buffer, buffer + i, bufferPos); /* slide remaining to front */
            i = -1;                                 /* restart for loop scan */
        }
    }

    if ((unsigned)bufferPos >= sizeof(buffer) - 1)
    {
        // log(fmt("Fifo overflow"));
        close();
        open();
    }
}

void Fifo::ioCb(ev::io &, int revents)
{
    if (EV_ERROR & revents)
    {
        int sockErrno = readFdError(this->fd);
        if (sockErrno)
        {
            // log(fmt("Error on fifo: %s\n", strerror(sockErrno)));
            close();
            open();
        }
    }
    else if (revents & EV_READ)
    {
        read();
    }
}

#endif
