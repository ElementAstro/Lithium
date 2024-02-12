# Atom-Event Module

## Description

This module is transformed from the libkev (a cross-platform C++(C++14) event loop). It is a high performance event loop library.

To enhance the performance of the Atom Driver Communication, we included this module and rewrote it.

Another reason is that the libkev has a good performance cross different platforms, including windows/mac/iOS/linux/android

## Usage

A simple example from the libkev[README](README.md)

```cpp
#include "kev.h"

#include <thread>
#include <memory>

using namespace Atom::Event;

void foo()
{
    printf("foo called\n");
}

void foo2()
{
    printf("foo2 called\n");
}

int bar()
{
    printf("bar called\n");
    return 333;
}

int main(int argc, const char * argv[])
{
    EventLoop run_loop;
    EventLoop::Token token = run_loop.createToken();

    std::thread thr([&] {
        if (run_loop.init()) {
            run_loop.loop();
        }
    });

    Timer timer(&run_loop);
    timer.schedule(3000, Timer::Mode::ONE_SHOT, [&] {
        printf("onTimer\n");

        run_loop.stop();
    });

    auto delayed_token = run_loop.createToken();
    run_loop.postDelayed(1000,[] {
        printf("postDelayed 1000\n");
    }, &delayed_token);

    run_loop.postDelayed(1500,[] {
        printf("postDelayed 1500\n");
    });

    run_loop.postDelayed(5000,[] {
        printf("postDelayed 5000\n");
    });

    delayed_token.reset();

    run_loop.async([] { printf ("loop async\n"); }, &token);
    printf("async called\n");

    auto ret = run_loop.invoke([] { printf ("loop invoke\n"); return 88; });
    printf("ret=%d\n", ret);

    auto int_ptr = std::make_unique<int>(123);

    ret = run_loop.invoke([p=std::move(int_ptr)] {
        return *p;
    });
    printf("move-only ret=%d\n", ret);

    int_ptr = std::make_unique<int>(456);
    run_loop.sync([p=std::move(int_ptr)] {
        printf("sync: move-only, i=%d\n", *p);
    });

    int_ptr = std::make_unique<int>(789);
    run_loop.async([p=std::move(int_ptr)] {
        printf("async: move-only, i=%d\n", *p);
    }, &token);

    run_loop.post(foo);
    int ddc = 81;
    run_loop.post([=] {
        printf("ddc=%d\n", ddc);
        foo2();
    }, &token);

    //token.reset();

    ret = run_loop.invoke(bar);
    printf("ret=%d\n", ret);

    run_loop.invoke(foo);

    try {
        if (thr.joinable()) {
            thr.join();
        }
    } catch(...) {}

    return 0;
}
```

## Requirements

- A conforming C++14 compiler, however , after we rewrite the libkev, the c++20 is required.
- CMake v3.20 or newer
