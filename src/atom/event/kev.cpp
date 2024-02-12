/*
 * kev.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-10

Description: Main

**************************************************/

#include "kev.hpp"
#include "eventloop.hpp"
#include "utils/kmtrace.hpp"

ATOM_NS_BEGIN

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// class EventLoop
EventLoop::EventLoop()
    : EventLoop(PollType::DEFAULT)
{
}

EventLoop::EventLoop(PollType poll_type)
    : pimpl_(std::make_shared<Impl>(poll_type))
{
}

EventLoop::EventLoop(EventLoop &&other)
    : pimpl_(std::exchange(other.pimpl_, nullptr))
{
}

EventLoop::~EventLoop()
{
}

EventLoop &EventLoop::operator=(EventLoop &&other)
{
    if (this != &other)
    {
        pimpl_ = std::exchange(other.pimpl_, nullptr);
    }

    return *this;
}

bool EventLoop::init()
{
    return pimpl_->init();
}

EventLoop::Token EventLoop::createToken()
{
    Token t;
    if (!t.pimpl_)
    { // lazy initialize token pimpl
        t.pimpl_ = new Token::Impl();
    }
    t.pimpl()->eventLoop(pimpl());
    return t;
}

PollType EventLoop::getPollType() const
{
    return pimpl_->getPollType();
}

bool EventLoop::isPollLT() const
{
    return pimpl_->isPollLT();
}

Result EventLoop::registerFd(SOCKET_FD fd, uint32_t events, IOCallback cb)
{
    return pimpl_->registerFd(fd, events, std::move(cb));
}

Result EventLoop::updateFd(SOCKET_FD fd, uint32_t events)
{
    return pimpl_->updateFd(fd, events);
}

Result EventLoop::unregisterFd(SOCKET_FD fd, bool close_fd)
{
    return pimpl_->unregisterFd(fd, close_fd);
}

void EventLoop::loopOnce(uint32_t max_wait_ms)
{
    pimpl_->loopOnce(max_wait_ms);
}

void EventLoop::loop(uint32_t max_wait_ms)
{
    pimpl_->loop(max_wait_ms);
}

void EventLoop::stop()
{
    pimpl_->stop();
}

bool EventLoop::stopped() const
{
    return pimpl_->stopped();
}

void EventLoop::reset()
{
    pimpl_->reset();
}

EventLoop::ImplPtr EventLoop::pimpl()
{
    return pimpl_;
}

bool EventLoop::inSameThread() const
{
    return pimpl_->inSameThread();
}

Result EventLoop::sync(Task task, Token *token, const char *debugStr)
{
    return pimpl_->sync(std::move(task), token ? token->pimpl() : nullptr, debugStr);
}

Result EventLoop::async(Task task, Token *token, const char *debugStr)
{
    return pimpl_->async(std::move(task), token ? token->pimpl() : nullptr, debugStr);
}

Result EventLoop::post(Task task, Token *token, const char *debugStr)
{
    return pimpl_->post(std::move(task), token ? token->pimpl() : nullptr, debugStr);
}

Result EventLoop::postDelayed(uint32_t delay_ms, Task task, Token *token, const char *debugStr)
{
    return pimpl_->postDelayed(delay_ms, std::move(task), token ? token->pimpl() : nullptr, debugStr);
}

void EventLoop::wakeup()
{
    pimpl_->wakeup();
}

void EventLoop::cancel(Token *token)
{
    if (token)
    {
        token->pimpl()->clearAllTasks();
    }
}

EventLoop::Token::Token()
    : pimpl_(nullptr) // lazy initialize pimpl_
{
}

EventLoop::Token::Token(Token &&other)
    : pimpl_(std::exchange(other.pimpl_, nullptr))
{
}

EventLoop::Token::~Token()
{
    delete pimpl_;
}

EventLoop::Token &EventLoop::Token::operator=(Token &&other)
{
    if (this != &other)
    {
        if (pimpl_)
        {
            delete pimpl_;
        }
        pimpl_ = std::exchange(other.pimpl_, nullptr);
    }
    return *this;
}

void EventLoop::Token::reset()
{
    if (pimpl_)
    {
        pimpl_->reset();
    }
}

EventLoop::Token::Impl *EventLoop::Token::pimpl()
{
    return pimpl_;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

Timer::Timer(EventLoop *loop)
    : pimpl_(new Impl(loop->pimpl()->getTimerMgr()))
{
}

Timer::Timer(Timer &&other)
    : pimpl_(std::exchange(other.pimpl_, nullptr))
{
}

Timer::~Timer()
{
    delete pimpl_;
}

Timer &Timer::operator=(Timer &&other)
{
    if (this != &other)
    {
        if (pimpl_)
        {
            delete pimpl_;
        }
        pimpl_ = std::exchange(other.pimpl_, nullptr);
    }

    return *this;
}

bool Timer::schedule(uint32_t delay_ms, Mode mode, TimerCallback cb)
{
    return pimpl_->schedule(delay_ms, mode, std::move(cb));
}

void Timer::cancel()
{
    pimpl_->cancel();
}

Timer::Impl *Timer::pimpl()
{
    return pimpl_;
}

void setLogCallback(LogCallback cb)
{
    setTraceFunc(cb);
}

ATOM_NS_END
