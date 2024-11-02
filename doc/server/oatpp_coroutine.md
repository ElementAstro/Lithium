# Oat++ 中的协程

Oat++ 中的协程不是普通的协程。  
Oat++ 实现了自定义的无状态协程，并带有调度功能。调度提供了优化的空间，并更好地利用了 CPU 资源。

Oat++ 中的协程通过 [oatpp::async::Executor](/api/latest/oatpp/core/async/Executor/) 执行。在每次迭代中，协程返回一个 [oatpp::async::Action](/api/latest/oatpp/core/async/Coroutine/#action)，告诉执行器下一步该做什么。  
根据 Action，Oat++ 异步处理器将协程重新调度到相应的 worker。

## 异步执行器

[oatpp::async::Executor](/api/latest/oatpp/core/async/Executor/) 分配了三组 worker，每组指定数量的线程。

```cpp
oatpp::async::Executor executor(
    1 /* 数据处理 worker */,
    1 /* I/O worker */,
    1 /* 定时器 worker */
);
```

所有协程最初都被放置在“数据处理” worker 组中，并可能根据协程迭代中返回的 [oatpp::async::Action](/api/latest/oatpp/core/async/Coroutine/#action) 重新调度到 I/O 或定时器 worker。

<img src="https://raw.githubusercontent.com/lganzzzo/oatpp-website-res/master/diagram/oatpp_async_executor.svg?sanitize=true" width="700px">

::: tip
尽管 Oat++ 异步处理器可能会将协程重新调度到不同的线程，但协程保证会在创建它的同一线程上被销毁。
:::

### I/O Worker

对于 I/O，`oatpp::async::Executor` 使用基于事件的 I/O 实现 [IOEventWorker](/api/latest/oatpp/core/async/worker/IOEventWorker/)：

- kqueue 实现 - 适用于 Mac/BSD 系统。
- epoll 实现 - 适用于 Linux 系统。

当协程返回类型为 [TYPE_IO_WAIT](/api/latest/oatpp/core/async/Coroutine/#action-type-io-wait) 的 Action 时，它将被重新调度到 I/O worker，并将文件描述符提供的 Action 放置到 kqueue/epoll 中。  
**因此，oatpp 协程不会浪费 CPU 资源来旋转和轮询长时间等待的连接。**

## API

在 oatpp 中，协程是从 [oatpp::async::Coroutine](/api/latest/oatpp/core/async/Coroutine/#coroutine) 或 [oatpp::async::CoroutineWithResult](/api/latest/oatpp/core/async/Coroutine/#coroutinewithresult) 扩展的类。  
协程在 [oatpp::async::Executor](/api/latest/oatpp/core/async/Executor/) 中处理。

```cpp
class MyCoroutine : public oatpp::async::Coroutine<MyCoroutine> {
public:

  /*
   * act() - 协程的入口点
   * 返回 Action - 下一步该做什么
   */
  Action act() override {
    OATPP_LOGD("MyCoroutine", "act()");
    return yieldTo(&MyCoroutine::step2);
  }

  Action step2() {
    OATPP_LOGD("MyCoroutine", "step2");
    return yieldTo(&MyCoroutine::step3);
  }

  Action step3() {
    OATPP_LOGD("MyCoroutine", "step3");
    return finish();
  }

};

oatpp::async::Executor executor();

executor.execute<MyCoroutine>();

executor.waitTasksFinished();
executor.stop();
executor.join();
```

输出：

```
MyCoroutine:act()
MyCoroutine:step2
MyCoroutine:step3
```

## 从协程调用协程

```cpp
class OtherCoroutine : public oatpp::async::Coroutine<OtherCoroutine> {
public:
  Action act() override {
    OATPP_LOGD("OtherCoroutine", "act()");
    return finish();
  }
};

class MyCoroutine : public oatpp::async::Coroutine<MyCoroutine> {
public:

  Action act() override {
    OATPP_LOGD("MyCoroutine", "act()");
    return OtherCoroutine::start().next(finish()); /* 在 OtherCoroutine 完成后执行的 Action */);
  }

};

oatpp::async::Executor executor();

executor.execute<MyCoroutine>();

executor.waitTasksFinished();
executor.stop();
executor.join();
```

输出：

```
MyCoroutine:act()
OtherCoroutine:act()
```

## 调用协程并返回结果

```cpp
class CoroutineWithResult : public oatpp::async::CoroutineWithResult<CoroutineWithResult, const char* /* 结果类型 */> {
public:
  Action act() override {
    OATPP_LOGD("CoroutineWithResult", "act()");
    return _return("<result>");
  }
};

class MyCoroutine : public oatpp::async::Coroutine<MyCoroutine> {
public:

  Action act() override {
    OATPP_LOGD("MyCoroutine", "act()");
    return CoroutineWithResult::startForResult().callbackTo(&MyCoroutine::onResult);
  }

  Action onResult(const char* result) {
    OATPP_LOGD("MyCoroutine", "result='%s'", result);
    return finish();
  }

};

oatpp::async::Executor executor();

executor.execute<MyCoroutine>();

executor.waitTasksFinished();
executor.stop();
executor.join();
```

输出：

```
MyCoroutine:act()
CoroutineWithResult:act()
MyCoroutine:result='<result>'
```

## 计数器

```cpp
class MyCoroutineCounter : public oatpp::async::Coroutine<MyCoroutineCounter> {
private:
  const char* m_name;
  v_int32 m_counter = 0;
public:

  MyCoroutineCounter(const char* name) : m_name(name) {}

  Action act() override {
    OATPP_LOGD(m_name, "counter=%d", m_counter);
    if(m_counter < 10) {
      m_counter ++;
      return repeat();
    }
    return finish();
  }

};

oatpp::async::Executor executor();

executor.execute<MyCoroutineCounter>("A");
executor.execute<MyCoroutineCounter>("B");
executor.execute<MyCoroutineCounter>("C");

executor.waitTasksFinished();
executor.stop();
executor.join();
```

可能的输出：

```
A:counter=0
B:counter=0
C:counter=0
A:counter=1
B:counter=1
C:counter=1
A:counter=2
B:counter=2
C:counter=2
A:counter=3
B:counter=3
C:counter=3
A:counter=4
B:counter=4
C:counter=4
A:counter=5
B:counter=5
C:counter=5
A:counter=6
B:counter=6
C:counter=6
A:counter=7
B:counter=7
C:counter=7
A:counter=8
B:counter=8
C:counter=8
A:counter=9
B:counter=9
C:counter=9
A:counter=10
B:counter=10
C:counter=10
```
