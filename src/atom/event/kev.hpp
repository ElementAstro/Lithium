/*
 * kev.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-10

Description: Main

**************************************************/

#ifndef __KEVAPI_HPP
#define __KEVAPI_HPP

#include "kevdefs.hpp"
#include "kmtypes.hpp"

#include <functional>
#include <memory>
#include <string>

ATOM_NS_BEGIN

class EventLoop
{
public:
    using Task = std::function<void(void)>;

    class Token
    {
    public:
        Token();
        Token(Token &&other);
        Token(const Token &other) = delete;
        ~Token();

        Token &operator=(Token &&other);
        Token &operator=(const Token &other) = delete;

        /* clear all tasks that are scheduled to this token. you cannot cancel the task that is in running,
         * but will wait untill the task completion
         */
        void reset();

        class Impl;
        Impl *pimpl();

    private:
        Impl *pimpl_;

        friend class EventLoop;
    };

public:
    EventLoop();
    EventLoop(PollType poll_type);
    EventLoop(const EventLoop &) = delete;
    EventLoop(EventLoop &&other);
    ~EventLoop();

    EventLoop &operator=(const EventLoop &) = delete;
    EventLoop &operator=(EventLoop &&other);

public:
    bool init();

    /* NOTE: cb must be valid untill unregisterFd called
     * this API is thread-safe
     */
    Result registerFd(SOCKET_FD fd, uint32_t events, IOCallback cb);
    /*
     * this API is thread-safe
     */
    Result updateFd(SOCKET_FD fd, uint32_t events);
    /*
     * this API is thread-safe
     */
    Result unregisterFd(SOCKET_FD fd, bool close_fd);

    PollType getPollType() const;
    bool isPollLT() const; // level trigger

public:
    bool inSameThread() const;

    /* create a token, it can be used to cancel the tasks that are scheduled with it
     * if caller can guarantee the resources used by tasks are valid when task running,
     * then token is no need, otherwise the caller should cancel the tasks queued in loop
     * before the resources are unavailable
     */
    Token createToken();

    /* run the task in loop thread and wait untill task is executed.
     * the task will be executed at once if called on loop thread
     * token is always unnecessary for sync task
     *
     * @param f the task to be executed. it will always be executed when call success
     *
     * @return return the result of f()
     */
    template <typename F>
    auto invoke(F &&f, Token *token = nullptr, const char *debugStr = nullptr)
    {
        Result err;
        return invoke(std::forward<F>(f), err, token, debugStr);
    }

    /* run the task in loop thread and wait untill task is executed.
     * the task will be executed at once if called on loop thread
     * token is always unnecessary for sync task
     *
     * @param f the task to be executed. it will always be executed when call success
     * @param err when f is executed, err is Result::OK, otherwise is Result
     *
     * @return return the result of f()
     */
    template <typename F, std::enable_if_t<!std::is_void<decltype(std::declval<F>()())>{}, int> = 0>
    auto invoke(F &&f, Result &err, Token *token = nullptr, const char *debugStr = nullptr)
    {
        static_assert(!std::is_void<decltype(f())>{}, "is void");
        if (inSameThread())
        {
            return f();
        }
        using ReturnType = decltype(f());
        ReturnType retval;
        auto task_sync = [&]
        { retval = f(); };
        err = sync(std::move(task_sync), token, debugStr);
        return retval;
    }

    template <typename F, std::enable_if_t<std::is_void<decltype(std::declval<F>()())>{}, int> = 0>
    void invoke(F &&f, Result &err, Token *token = nullptr, const char *debugStr = nullptr)
    {
        static_assert(std::is_void<decltype(f())>{}, "not void");
        if (inSameThread())
        {
            return f();
        }
        err = sync(std::forward<F>(f), token, debugStr);
    }

    /* run the task in loop thread and wait untill task is executed.
     * the task will be executed at once if called on loop thread
     * token is always unnecessary for sync task
     *
     * @param task the task to be executed. it will always be executed when call success
     */
    template <typename F, std::enable_if_t<!std::is_copy_constructible<F>{}, int> = 0>
    Result sync(F &&f, Token *token = nullptr, const char *debugStr = nullptr)
    {
        lambda_wrapper<F> wf{std::forward<F>(f)};
        return sync(Task(std::move(wf)), token, debugStr);
    }
    Result sync(Task task, Token *token = nullptr, const char *debugStr = nullptr);

    /* run the task in loop thread.
     * the task will be executed at once if called on loop thread
     *
     * @param task the task to be executed. it will always be executed when call success
     * @param token to be used to cancel the task. If token is null, the caller should
     *              make sure the resources referenced by task are valid when task running
     * @param debugStr debug message of the f, e.g. file name and line where f is generated
     */
    template <typename F, std::enable_if_t<!std::is_copy_constructible<F>{}, int> = 0>
    Result async(F &&f, Token *token = nullptr, const char *debugStr = nullptr)
    {
        lambda_wrapper<F> wf{std::forward<F>(f)};
        return async(Task(std::move(wf)), token, debugStr);
    }
    Result async(Task task, Token *token = nullptr, const char *debugStr = nullptr);

    /* run the task in loop thread at next time.
     *
     * @param task the task to be executed. it will always be executed when call success
     * @param token to be used to cancel the task. If token is null, the caller should
     *              make sure the resources referenced by task are valid when task running
     */
    template <typename F, std::enable_if_t<!std::is_copy_constructible<F>{}, int> = 0>
    Result post(F &&f, Token *token = nullptr, const char *debugStr = nullptr)
    {
        lambda_wrapper<F> wf{std::forward<F>(f)};
        return post(Task(std::move(wf)), token, debugStr);
    }
    Result post(Task task, Token *token = nullptr, const char *debugStr = nullptr);

    template <typename F, std::enable_if_t<!std::is_copy_constructible<F>{}, int> = 0>
    Result postDelayed(uint32_t delay_ms, F &&f, Token *token = nullptr, const char *debugStr = nullptr)
    {
        lambda_wrapper<F> wf{std::forward<F>(f)};
        return postDelayed(delay_ms, Task(std::move(wf)), token, debugStr);
    }
    Result postDelayed(uint32_t delay_ms, Task task, Token *token = nullptr, const char *debugStr = nullptr);

    void wakeup();

    /* cancel the tasks that are scheduled with token. you cannot cancel the task that is in running,
     * but will wait untill the task completion
     *
     * @param token token of the tasks
     * this API is thread-safe
     */
    void cancel(Token *token);

    void loopOnce(uint32_t max_wait_ms);
    void loop(uint32_t max_wait_ms = -1);

    /* stop the loop, no more Task can be posted to loop on stopped state
     * reset() can be used to reset the stopped falg
     */
    void stop();
    bool stopped() const;

    /* reset the loop state, Task can be posted again to the loop after reset
     */
    void reset();

    class Impl;
    using ImplPtr = std::shared_ptr<Impl>;
    ImplPtr pimpl();

private:
    ImplPtr pimpl_;
};

class Timer
{
public:
    using TimerCallback = std::function<void(void)>;
    enum class Mode
    {
        ONE_SHOT,
        REPEATING
    };

    Timer(EventLoop *loop);
    Timer(const Timer &) = delete;
    Timer(Timer &&other);
    ~Timer();

    Timer &operator=(const Timer &) = delete;
    Timer &operator=(Timer &&other);

    /**
     * Schedule the timer. This API is thread-safe
     */
    template <typename F, std::enable_if_t<!std::is_copy_constructible<F>{}, int> = 0>
    bool schedule(uint32_t delay_ms, Mode mode, F &&f)
    {
        lambda_wrapper<F> wf{std::forward<F>(f)};
        return schedule(delay_ms, mode, TimerCallback(std::move(wf)));
    }
    // TimerCallback will be reset when:
    // 1. timer is cancelled
    // 2. timer is executed and Timer::Mode is ONE_SHOT
    // 3. the TimerManager in EventLoop is destroyed and timer is still in queue
    bool schedule(uint32_t delay_ms, Mode mode, TimerCallback cb);

    /**
     * Cancel the scheduled timer. This API is thread-safe
     */
    void cancel();

    class Impl;
    Impl *pimpl();

private:
    Impl *pimpl_;
};

using LogCallback = std::function<void(int level, std::string &&msg)>;
void setLogCallback(LogCallback cb);

ATOM_NS_END

#endif
