#include "local_driver.hpp"

#include <unistd.h>
#include <fcntl.h>

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#else
#include <sys/socket.h>
#include <libgen.h>
#include <netdb.h>
#include <netinet/in.h>
#endif

#include "io.hpp"
#include "lilxml.hpp"
#include "client_info.hpp"
#include "xml_util.hpp"
#include "hydrogen_server.hpp"

#include "atom/log/loguru.hpp"

LocalDvrInfo::LocalDvrInfo() : DvrInfo(true)
{
#ifdef USE_LIBUV
    eio.data = this;
    pidwatcher.data = this;
#else
    eio.set<LocalDvrInfo, &LocalDvrInfo::onEfdEvent>(this);
    pidwatcher.set<LocalDvrInfo, &LocalDvrInfo::onPidEvent>(this);
#endif
}

LocalDvrInfo::LocalDvrInfo(const LocalDvrInfo &model) : DvrInfo(model),
                                                        envDev(model.envDev),
                                                        envConfig(model.envConfig),
                                                        envSkel(model.envSkel),
                                                        envPrefix(model.envPrefix)
{
#ifdef USE_LIBUV
    eio.data = this;
    pidwatcher.data = this;
#else
    eio.set<LocalDvrInfo, &LocalDvrInfo::onEfdEvent>(this);
    pidwatcher.set<LocalDvrInfo, &LocalDvrInfo::onPidEvent>(this);
#endif
}

LocalDvrInfo::~LocalDvrInfo()
{
    closeEfd();
    if (pid != 0)
    {
        kill(pid, SIGKILL); /* libev insures there will be no zombies */
        pid = 0;
    }
    closePid();
}

/* start the given local HYDROGEN driver process.
 * exit if trouble.
 */
#ifdef _WIN32
void LocalDvrInfo::start()
{
    Msg *mp;
    HANDLE hReadPipe, hWritePipe, hErrorPipe;
    SECURITY_ATTRIBUTES sa;
    PROCESS_INFORMATION pi;
    STARTUPINFO si;
    HANDLE hEfd;
    DWORD pid;

#ifdef OSX_EMBEDED_MODE
    fprintf(stderr, "STARTING \"%s\"\n", name.c_str());
    fflush(stderr);
#endif

    HANDLE hRp, hWp, hEp; // 修改变量声明为HANDLE类型

    /* build three pipes: r, w and error*/
    if (useSharedBuffer)
    {
        // FIXME: lots of FD are opened by hydrogenserver. FD_CLOEXEC is a must + check other fds
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.bInheritHandle = TRUE;
        sa.lpSecurityDescriptor = NULL;
        if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0))
        {
            log(fmt::format("CreatePipe: %d\n", GetLastError()));
            // Bye();
        }
        hErrorPipe = hWritePipe;
    }
    else
    {

        if (!CreatePipe(&hRp, &hWp, NULL, 4096))
        {
            log(fmt::format("CreatePipe read: %d\n", GetLastError()));
            // Bye();
        }
        if (!CreatePipe(&hRp, &hEp, NULL, 4096))
        {
            log(fmt::format("CreatePipe error: %d\n", GetLastError()));
            // Bye();
        }
        hReadPipe = hRp;
        hWritePipe = hWp;
        hErrorPipe = hEp;
    }

    /* fork&exec new process */
    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    si.hStdInput = hReadPipe;
    si.hStdOutput = hWritePipe;
    si.hStdError = hErrorPipe;
    si.dwFlags |= STARTF_USESTDHANDLES;

    if (!CreateProcess(NULL, (LPSTR)name.c_str(), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
    {
        log(fmt::format("CreateProcess: %d\n", GetLastError()));
        // Bye();
    }

    if (useSharedBuffer)
    {
        /* don't need child's other socket end */
        CloseHandle(hReadPipe);

        /* record pid, io channels, init lp and snoop list */
        setFds(reinterpret_cast<intptr_t>(hWritePipe), reinterpret_cast<intptr_t>(hWritePipe));
    }
    else
    {
        /* don't need child's side of pipes */
        CloseHandle(hWritePipe);
        CloseHandle(hReadPipe);

        /* record pid, io channels, init lp and snoop list */
        setFds(reinterpret_cast<intptr_t>(int) hRp, reinterpret_cast<intptr_t>(int) hWp); // 修改为使用新的变量名
    }

    CloseHandle(hErrorPipe);

    // Watch pid
    this->pid = pi.dwProcessId;
    do
    {
        (this->pidwatcher).pid = (this->pid);
        (this->pidwatcher).flags = !!(1);
    } while (0);
    ev_child_set() this->pidwatcher.set(this->pid);
    // this->pidwatcher.start();

    // Watch input on efd
    hEfd = hEp; // 修改为使用新的变量名
    SetHandleInformation(hEfd, HANDLE_FLAG_INHERIT, 0);
    this->efd = _open_osfhandle(reinterpret_cast<intptr_t>(hEfd), _O_RDONLY | _O_BINARY);
    unsigned long mode = 1;
    ioctlsocket(this->efd, FIONBIO, &mode);
    this->eio.start(this->efd, ev::READ);

    /* first message primes driver to report its properties -- dev known
     * if restarting
     */
    if (verbose > 0)
        DLOG_F(INFO, "pid=%d rfd=%d wfd=%d efd=%d\n", pid, (int)hRp, (int)hWp, (int)hEp);

    XMLEle *root = addXMLEle(NULL, "getProperties");
    addXMLAtt(root, "version", TO_STRING(HYDROGENV));
    mp = new Msg(nullptr, root);

    // pushmsg can kill mp. do at end
    pushMsg(mp);
}

#else
void LocalDvrInfo::start()
{
    Msg *mp;
    int rp[2], wp[2], ep[2];
    int ux[2];
    int pid;

#ifdef OSX_EMBEDED_MODE
    fprintf(stderr, "STARTING \"%s\"\n", name.c_str());
    fflush(stderr);
#endif

    /* build three pipes: r, w and error*/
    if (useSharedBuffer)
    {
        // FIXME: lots of FD are opened by hydrogenserver. FD_CLOEXEC is a must + check other fds
        if (socketpair(AF_UNIX, SOCK_STREAM, 0, ux) == -1)
        {
            log(fmt::format("socketpair: %s\n", strerror(errno)));
            // Bye();
        }
    }
    else
    {
        if (pipe(rp) < 0)
        {
            log(fmt::format("read pipe: %s\n", strerror(errno)));
            // Bye();
        }
        if (pipe(wp) < 0)
        {
            log(fmt::format("write pipe: %s\n", strerror(errno)));
            // Bye();
        }
    }
    if (pipe(ep) < 0)
    {
        log(fmt::format("stderr pipe: %s\n", strerror(errno)));
        // Bye();
    }

    /* fork&exec new process */
    pid = fork();
    if (pid < 0)
    {
        log(fmt::format("fork: %s\n", strerror(errno)));
        // Bye();
    }
    if (pid == 0)
    {
        /* child: exec name */
        int fd;

        /* rig up pipes */
        if (useSharedBuffer)
        {
            // For unix socket, the same socket end can be used for both read & write
            dup2(ux[0], 0); /* driver stdin reads from ux[0] */
            dup2(ux[0], 1); /* driver stdout writes to ux[0] */
            ::close(ux[0]);
            ::close(ux[1]);
        }
        else
        {
            dup2(wp[0], 0); /* driver stdin reads from wp[0] */
            dup2(rp[1], 1); /* driver stdout writes to rp[1] */
        }
        dup2(ep[1], 2); /* driver stderr writes to e[]1] */
        for (fd = 3; fd < 100; fd++)
            (void)::close(fd);

        if (!envDev.empty())
            setenv("HYDROGENDEV", envDev.c_str(), 1);
        /* Only reset environment variable in case of FIFO */
        else if (fifo)
            unsetenv("HYDROGENDEV");
        if (!envConfig.empty())
            setenv("HYDROGENCONFIG", envConfig.c_str(), 1);
        else if (fifo)
            unsetenv("HYDROGENCONFIG");
        if (!envSkel.empty())
            setenv("HYDROGENSKEL", envSkel.c_str(), 1);
        else if (fifo)
            unsetenv("HYDROGENSKEL");
        std::string executable;
        if (!envPrefix.empty())
        {
            setenv("HYDROGENPREFIX", envPrefix.c_str(), 1);
#if defined(OSX_EMBEDED_MODE)
            executable = envPrefix + "/Contents/MacOS/" + name;
#elif defined(__APPLE__)
            executable = envPrefix + "/" + name;
#else
            executable = envPrefix + "/bin/" + name;
#endif

            fprintf(stderr, "%s\n", executable.c_str());

            execlp(executable.c_str(), name.c_str(), NULL);
        }
        else
        {
            if (name[0] == '.')
            {
                executable = std::string(dirname((char *)me)) + "/" + name;
                execlp(executable.c_str(), name.c_str(), NULL);
            }
            else
            {
                execlp(name.c_str(), name.c_str(), NULL);
            }
        }

#ifdef OSX_EMBEDED_MODE
        fprintf(stderr, "FAILED \"%s\"\n", name.c_str());
        fflush(stderr);
#endif
        log(fmt::format("execlp %s: %s\n", executable.c_str(), strerror(errno)));
        _exit(1); /* parent will notice EOF shortly */
    }

    if (useSharedBuffer)
    {
        /* don't need child's other socket end */
        ::close(ux[0]);

        /* record pid, io channels, init lp and snoop list */
        setFds(ux[1], ux[1]);
        rp[0] = ux[1];
        wp[1] = ux[1];
    }
    else
    {
        /* don't need child's side of pipes */
        ::close(wp[0]);
        ::close(rp[1]);

        /* record pid, io channels, init lp and snoop list */
        setFds(rp[0], wp[1]);
    }

    ::close(ep[1]);

    // Watch pid
    this->pid = pid;
#ifdef USE_LIBUV
    if (this->pidwatcher.loop != nullptr && this->pidwatcher.signal_cb != nullptr)
    {
        uv_signal_start(&this->pidwatcher, this->pidwatcher.signal_cb, SIGCHLD);
    }

#else
    this->pidwatcher.set(pid);
    this->pidwatcher.start();
#endif

    // Watch input on efd
    this->efd = ep[0];
    fcntl(this->efd, F_SETFL, fcntl(this->efd, F_GETFL, 0) | O_NONBLOCK);
#ifdef USE_LIBUV
    // Start eio file descriptor watcher
    if (this->eio.loop != nullptr)
    {
        uv_poll_start(&this->eio, UV_READABLE, (uv_poll_cb)&LocalDvrInfo::onEfdEvent);
    }

#else
    this->eio.start(this->efd, ev::READ);
#endif

    /* first message primes driver to report its properties -- dev known
     * if restarting
     */
    if (verbose > 0)
        DLOG_F(INFO, "pid=%d rfd=%d wfd=%d efd=%d\n", pid, rp[0], wp[1], ep[0]);

    XMLEle *root = addXMLEle(NULL, "getProperties");
    addXMLAtt(root, "version", TO_STRING(HYDROGENV));
    mp = new Msg(nullptr, root);

    // pushmsg can kill mp. do at end
    pushMsg(mp);
}
#endif

LocalDvrInfo *LocalDvrInfo::clone() const
{
    return new LocalDvrInfo(*this);
}

void LocalDvrInfo::closeEfd()
{
    ::close(efd);
    efd = -1;
#ifdef USE_LIBUV
    uv_poll_stop(&eio);
#else
    eio.stop();
#endif
}

void LocalDvrInfo::closePid()
{
    pid = 0;
#ifdef USE_LIBUV
    uv_signal_stop(&pidwatcher);
#else
    pidwatcher.stop();
#endif
}

#ifdef USE_LIBUV

void LocalDvrInfo::onEfdEvent(uv_poll_t *handle, int status, int events)
{
    if (status < 0)
    {
        // 处理错误情况
        const char *error = uv_strerror(status);
        LOG_F(ERROR, "Error on stderr: {}", error);
        closeEfd();
        return;
    }

    if (events & UV_READABLE)
    {
        ssize_t nr;

        /* read more */
        nr = read(efd, errbuff + errbuffpos, sizeof(errbuff) - errbuffpos);
        if (nr <= 0)
        {
            if (nr < 0)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                    return;

                LOG_F(ERROR, "stderr {}", strerror(errno));
            }
            else
                LOG_F(ERROR, "stderr EOF");
            closeEfd();
            return;
        }
        errbuffpos += nr;

        for (int i = 0; i < errbuffpos; ++i)
        {
            if (errbuff[i] == '\n')
            {
                LOG_F(ERROR, "{}{}", (int)i, errbuff);
                i++;                                       /* count including nl */
                errbuffpos -= i;                           /* remove from nexbuf */
                memmove(errbuff, errbuff + i, errbuffpos); /* slide remaining to front */
                i = -1;                                    /* restart for loop scan */
            }
        }
    }
}

#ifdef _WIN32
void LocalDvrInfo::onPidEvent(uv_signal_t *handle, int exit_status)
#else
void LocalDvrInfo::onPidEvent(uv_process_t *handle, int exit_status, int term_signal)
#endif
{
    // 处理pid事件
    if (exit_status == UV_SIGNAL && handle->status)
    {
        if (WIFEXITED(pidwatcher.signum))
        {
            LOG_F(ERROR, "process {} exited with status {}", pid, WEXITSTATUS(pidwatcher.signum));
        }
        else if (WIFSIGNALED(pidwatcher.signum))
        {
            int exit_status = WTERMSIG(pidwatcher.signum);
            LOG_F(ERROR, "process {} killed with signal {} - {}", pid, exit_status, strsignal(exit_status));
        }
        pid = 0;
        uv_signal_stop(&pidwatcher);
    }
}

#else
void LocalDvrInfo::onEfdEvent(ev::io &, int revents)
{
    if (EV_ERROR & revents)
    {
        int sockErrno = readFdError(this->efd);
        if (sockErrno)
        {
            LOG_F(ERROR, "Error on stderr: {}", strerror(sockErrno));
            closeEfd();
        }
        return;
    }

    if (revents & EV_READ)
    {
        ssize_t nr;

        /* read more */
        nr = read(efd, errbuff + errbuffpos, sizeof(errbuff) - errbuffpos);
        if (nr <= 0)
        {
            if (nr < 0)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                    return;

                LOG_F(ERROR, "stderr {}", strerror(errno));
            }
            else
                LOG_F(ERROR, "stderr EOF");
            closeEfd();
            return;
        }
        errbuffpos += nr;

        for (int i = 0; i < errbuffpos; ++i)
        {
            if (errbuff[i] == '\n')
            {
                LOG_F(ERROR, "{}{}", (int)i, errbuff);
                i++;                                       /* count including nl */
                errbuffpos -= i;                           /* remove from nexbuf */
                memmove(errbuff, errbuff + i, errbuffpos); /* slide remaining to front */
                i = -1;                                    /* restart for loop scan */
            }
        }
    }
}

#ifdef _WIN32
void LocalDvrInfo::onPidEvent(ev_child &, int revents)
#else
void LocalDvrInfo::onPidEvent(ev::child &, int revents)
#endif
{
    if (revents & EV_CHILD)
    {
        if (WIFEXITED(pidwatcher.rstatus))
        {
            LOG_F(ERROR, "process {} exited with status {}", pid, WEXITSTATUS(pidwatcher.rstatus));
        }
        else if (WIFSIGNALED(pidwatcher.rstatus))
        {
            int exit_status = WTERMSIG(pidwatcher.rstatus);
            LOG_F(ERROR, "process {} killed with signal {} - {}", pid, exit_status, strsignal(exit_status));
        }
        pid = 0;
        this->pidwatcher.stop();
    }
}
#endif