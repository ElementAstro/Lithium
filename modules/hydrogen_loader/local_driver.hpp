#include "driver_info.hpp"

class LocalDvrInfo : public DvrInfo
{
    char errbuff[1024]; /* buffer for stderr pipe. line too long will be clipped */
    int errbuffpos = 0; /* first free pos in buffer */
#ifdef USE_LIBUV
    uv_poll_t eio;                                              // 替换 ev::io eio;
    void onEfdEvent(uv_poll_t *handle, int status, int events); // 替换 void onEfdEvent(ev::io &watcher, int revents);
    uv_signal_t pidwatcher;                                                 // 替换 ev::child pidwatcher;
    void onPidEvent(uv_process_t *handle, int exit_status, int term_signal); // 替换 void onPidEvent(ev_child &watcher, int revents);
#else
    ev::io eio;                                    /* Event loop io events */
    void onEfdEvent(ev::io &watcher, int revents); /* callback for data on efd */

#ifdef _WIN32
    ev_child pidwatcher;
#else
    ev::child pidwatcher;
#endif

#ifdef _WIN32
    void onPidEvent(ev_child &watcher, int revents);
#else
    void onPidEvent(ev::child &watcher, int revents);
#endif
#endif

    int pid = 0;  /* process id or 0 for N/A (not started/terminated) */
    int efd = -1; /* stderr from driver, or -1 when N/A */

    void closeEfd();
    void closePid();

protected:
    LocalDvrInfo(const LocalDvrInfo &model);

public:
    std::string envDev;
    std::string envConfig;
    std::string envSkel;
    std::string envPrefix;

    LocalDvrInfo();
    virtual ~LocalDvrInfo();

    virtual void start();

    virtual LocalDvrInfo *clone() const;

    virtual const std::string remoteServerUid() const
    {
        return "";
    }
};