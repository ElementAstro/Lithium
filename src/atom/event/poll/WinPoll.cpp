/*
 * winpoll.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-10

Description: WinPoll Notifier (Windows)

*************************************************/

#include "IOPoll.h"
#include "utils/kmtrace.h"

ATOM_NS_BEGIN

#define WM_SOCKET_NOTIFY 0x0373

#define WM_POLLER_NOTIFY WM_USER + 101

#define KM_WIN_CLASS_NAME "kev_win_class_name"

class WinPoll : public IOPoll
{
public:
    WinPoll();
    ~WinPoll();

    bool init();
    Result registerFd(SOCKET_FD fd, uint32_t events, IOCallback cb);
    Result unregisterFd(SOCKET_FD fd);
    Result updateFd(SOCKET_FD fd, uint32_t events);
    Result wait(uint32_t wait_ms);
    void notify();
    PollType getType() const { return PollType::WIN; }
    bool isLevelTriggered() const { return false; }

public:
    void on_socket_notify(SOCKET_FD fd, uint32_t events);

    void on_poller_notify();

private:
    uint32_t get_events(uint32_t kuma_events);
    uint32_t get_kuma_events(uint32_t events);
    void resizePollItems(SOCKET_FD fd);

private:
    HWND hwnd_;
    PollItemVector poll_items_;
};

WinPoll::WinPoll()
    : hwnd_(NULL)
{
}

WinPoll::~WinPoll()
{
    if (hwnd_)
    {
        if (::IsWindow(hwnd_))
        {
            DestroyWindow(hwnd_);
        }
        hwnd_ = NULL;
    }
}

bool WinPoll::init()
{
    hwnd_ = ::CreateWindow(KM_WIN_CLASS_NAME, NULL, WS_OVERLAPPED, 0,
                           0, 0, 0, NULL, NULL, NULL, 0);
    if (NULL == hwnd_)
    {
        return false;
    }
    SetWindowLong(hwnd_, 0, (LONG)this);
    return true;
}

uint32_t WinPoll::get_events(uint32_t kuma_events)
{
    uint32_t ev = 0;
    if (kuma_events & kEventRead)
    {
        ev |= FD_READ;
    }
    if (kuma_events & kEventWrite)
    {
        ev |= FD_WRITE;
    }
    if (kuma_events & kEventError)
    {
        ev |= FD_CLOSE;
    }
    return ev;
}

uint32_t WinPoll::get_kuma_events(uint32_t events)
{
    uint32_t ev = 0;
    if (events & FD_CONNECT)
    { // writeable
        ev |= kEventWrite;
    }
    if (events & FD_ACCEPT)
    { // writeable
        ev |= kEventRead;
    }
    if (events & FD_READ)
    {
        ev |= kEventRead;
    }
    if (events & FD_WRITE)
    {
        ev |= kEventWrite;
    }
    if (events & FD_CLOSE)
    {
        ev |= kEventError;
    }
    return ev;
}

void WinPoll::resizePollItems(SOCKET_FD fd)
{
    if (fd >= poll_items_.size())
    {
        poll_items_.resize(fd + 1);
    }
}

Result WinPoll::registerFd(SOCKET_FD fd, uint32_t events, IOCallback cb)
{
    KM_INFOTRACE("WinPoll::registerFd, fd=" << fd << ", events=" << events);
    resizePollItems(fd);
    poll_items_[fd].fd = fd;
    poll_items_[fd].cb = std::move(cb);
    WSAAsyncSelect(fd, hwnd_, WM_SOCKET_NOTIFY, get_events(events) | FD_CONNECT);
    return Result::OK;
}

Result WinPoll::unregisterFd(SOCKET_FD fd)
{
    KM_INFOTRACE("WinPoll::unregisterFd, fd=" << fd);
    SOCKET_FD max_fd = poll_items_.size() - 1;
    if (fd < 0 || -1 == max_fd || fd > max_fd)
    {
        KM_WARNTRACE("WinPoll::unregisterFd, failed, max_fd=" << max_fd);
        return Result::INVALID_PARAM;
    }
    if (fd == max_fd)
    {
        poll_items_.pop_back();
    }
    else
    {
        poll_items_[fd].cb = nullptr;
        poll_items_[fd].fd = INVALID_FD;
    }
    WSAAsyncSelect(fd, hwnd_, 0, 0);
    return Result::OK;
}

Result WinPoll::updateFd(SOCKET_FD fd, uint32_t events)
{
    SOCKET_FD max_fd = poll_items_.size() - 1;
    if (fd < 0 || -1 == max_fd || fd > max_fd)
    {
        KM_WARNTRACE("WinPoll::updateFd, failed, fd=" << fd << ", max_fd=" << max_fd);
        return Result::INVALID_PARAM;
    }
    if (poll_items_[fd].fd != fd)
    {
        KM_WARNTRACE("WinPoll::updateFd, failed, fd=" << fd << ", fd1=" << poll_items_[fd].fd);
        return Result::INVALID_PARAM;
    }
    return Result::OK;
}

Result WinPoll::wait(uint32_t wait_ms)
{
    MSG msg;
    if (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return Result::OK;
}

void WinPoll::notify()
{
    if (hwnd_)
    {
        ::PostMessage(hwnd_, WM_POLLER_NOTIFY, 0, 0);
    }
}

void WinPoll::on_socket_notify(SOCKET_FD fd, uint32_t events)
{
    int err = WSAGETSELECTERROR(events);
    int evt = WSAGETSELECTEVENT(events);
}

void WinPoll::on_poller_notify()
{
}

IOPoll *createWinPoll()
{
    return new WinPoll();
}

//////////////////////////////////////////////////////////////////////////
//
LRESULT CALLBACK km_notify_wnd_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_SOCKET_NOTIFY:
    {
        WinPoll *poll = (WinPoll *)GetWindowLong(hwnd, 0);
        if (poll)
            poll->on_socket_notify(wParam, lParam);
        return 0L;
    }

    case WM_POLLER_NOTIFY:
    {
        WinPoll *poll = (WinPoll *)GetWindowLong(hwnd, 0);
        if (poll)
            poll->on_poller_notify();
        return 0L;
    }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

static void initWinClass()
{
    WNDCLASS wc = {0};
    wc.style = 0;
    wc.lpfnWndProc = (WNDPROC)km_notify_wnd_proc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = sizeof(void *);
    wc.hInstance = NULL;
    wc.hIcon = 0;
    wc.hCursor = 0;
    wc.hbrBackground = 0;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = KM_WIN_CLASS_NAME;
    RegisterClass(&wc);
}

static void uninitWinClass()
{
    UnregisterClass(KM_WIN_CLASS_NAME, NULL);
}

// WBX_Init_Object g_init_obj(poller_load, poller_unload);

ATOM_NS_END
