#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <asio/io_context.hpp>

#include "atom/async/limiter.hpp"
#include "atom/async/message_bus.hpp"
#include "atom/async/message_queue.hpp"
#include "atom/async/pool.hpp"
#include "atom/async/safetype.hpp"
#include "atom/async/timer.hpp"
#include "atom/async/trigger.hpp"

namespace py = pybind11;
using namespace atom::async;

template <typename T>
void bind_message_queue(py::module &m, const std::string &name) {
    py::class_<MessageQueue<T>>(m, name.c_str())
        .def(py::init<asio::io_context &>(), "Constructor",
             py::arg("io_context"))
        .def("subscribe", &MessageQueue<T>::subscribe,
             "Subscribe to messages with a callback and optional filter and "
             "timeout",
             py::arg("callback"), py::arg("subscriber_name"),
             py::arg("priority") = 0, py::arg("filter") = nullptr,
             py::arg("timeout") = std::chrono::milliseconds::zero())
        .def("unsubscribe", &MessageQueue<T>::unsubscribe,
             "Unsubscribe from messages using the given callback",
             py::arg("callback"))
        .def("publish", &MessageQueue<T>::publish,
             "Publish a message to the queue, with an optional priority",
             py::arg("message"), py::arg("priority") = 0)
        .def("start_processing", &MessageQueue<T>::startProcessing,
             "Start processing messages in the queue")
        .def("stop_processing", &MessageQueue<T>::stopProcessing,
             "Stop processing messages in the queue")
        .def("get_message_count", &MessageQueue<T>::getMessageCount,
             "Get the number of messages currently in the queue")
        .def("get_subscriber_count", &MessageQueue<T>::getSubscriberCount,
             "Get the number of subscribers currently subscribed to the queue")
        .def("cancel_messages", &MessageQueue<T>::cancelMessages,
             "Cancel specific messages that meet a given condition",
             py::arg("cancel_condition"));
}

template <typename ParamType>
void bind_trigger(py::module &m, const std::string &name) {
    using TriggerType = Trigger<ParamType>;
    py::class_<TriggerType>(m, name.c_str())
        .def(py::init<>())
        .def("registerCallback", &TriggerType::registerCallback,
             py::arg("event"), py::arg("callback"),
             py::arg("priority") = TriggerType::CallbackPriority::Normal)
        .def("unregisterCallback", &TriggerType::unregisterCallback,
             py::arg("event"), py::arg("callback"))
        .def("trigger", &TriggerType::trigger, py::arg("event"),
             py::arg("param"))
        .def("scheduleTrigger", &TriggerType::scheduleTrigger, py::arg("event"),
             py::arg("param"), py::arg("delay"))
        .def("scheduleAsyncTrigger", &TriggerType::scheduleAsyncTrigger,
             py::arg("event"), py::arg("param"))
        .def("cancelTrigger", &TriggerType::cancelTrigger, py::arg("event"))
        .def("cancelAllTriggers", &TriggerType::cancelAllTriggers);

    py::enum_<typename TriggerType::CallbackPriority>(
        m, (name + "CallbackPriority").c_str())
        .value("High", TriggerType::CallbackPriority::High)
        .value("Normal", TriggerType::CallbackPriority::Normal)
        .value("Low", TriggerType::CallbackPriority::Low);
}

template <typename T>
void bind_safe_type(py::module &m, const std::string &name) {
    py::class_<LockFreeStack<T>>(m,
                                 std::format("LockFreeStack{}", name).c_str())
        .def(py::init<>())
        .def("push",
             (void(LockFreeStack<T>::*)(const T &)) & LockFreeStack<T>::push)
        .def("push", (void(LockFreeStack<T>::*)(T &&)) & LockFreeStack<T>::push)
        .def("pop", &LockFreeStack<T>::pop)
        .def("top", &LockFreeStack<T>::top)
        .def("empty", &LockFreeStack<T>::empty)
        .def("size", &LockFreeStack<T>::size);

    py::class_<ThreadSafeVector<T>>(
        m, std::format("ThreadSafeVector{}", name).c_str())
        .def(py::init<size_t>())
        .def("pushBack", (void(ThreadSafeVector<T>::*)(const T &)) &
                             ThreadSafeVector<T>::pushBack)
        .def("pushBack", (void(ThreadSafeVector<T>::*)(T &&)) &
                             ThreadSafeVector<T>::pushBack)
        .def("popBack", &ThreadSafeVector<T>::popBack)
        .def("at", &ThreadSafeVector<T>::at)
        .def("empty", &ThreadSafeVector<T>::empty)
        .def("getSize", &ThreadSafeVector<T>::getSize)
        .def("getCapacity", &ThreadSafeVector<T>::getCapacity)
        .def("clear", &ThreadSafeVector<T>::clear)
        .def("shrinkToFit", &ThreadSafeVector<T>::shrinkToFit)
        .def("front", &ThreadSafeVector<T>::front)
        .def("back", &ThreadSafeVector<T>::back)
        .def("__getitem__", &ThreadSafeVector<T>::operator[]);

    py::class_<LockFreeList<T>>(m, std::format("LockFreeList{}", name).c_str())
        .def(py::init<>())
        .def("pushFront", &LockFreeList<T>::pushFront)
        .def("popFront", &LockFreeList<T>::popFront)
        .def("empty", &LockFreeList<T>::empty);
}

PYBIND11_MODULE(async, m) {
    py::class_<MessageBus, std::shared_ptr<MessageBus>>(m, "MessageBus")
        .def(py::init<asio::io_context &>(), "Constructor",
             py::arg("io_context"))
        .def_static("create_shared", &MessageBus::createShared,
                    "Create a shared instance of MessageBus",
                    py::arg("io_context"))
        .def(
            "publish",
            [](MessageBus &self, const std::string &name,
               const py::object &message,
               std::optional<std::chrono::milliseconds> delay) {
                if (py::isinstance<py::str>(message)) {
                    self.publish(name, message.cast<std::string>(), delay);
                } else if (py::isinstance<py::int_>(message)) {
                    self.publish(name, message.cast<int>(), delay);
                } else if (py::isinstance<py::float_>(message)) {
                    self.publish(name, message.cast<double>(), delay);
                } else {
                    throw std::runtime_error("Unsupported message type");
                }
            },
            "Publish a message to the bus", py::arg("name"), py::arg("message"),
            py::arg("delay") = std::nullopt)
        .def(
            "publish_global",
            [](MessageBus &self, const py::object &message) {
                if (py::isinstance<py::str>(message)) {
                    self.publishGlobal(message.cast<std::string>());
                } else if (py::isinstance<py::int_>(message)) {
                    self.publishGlobal(message.cast<int>());
                } else if (py::isinstance<py::float_>(message)) {
                    self.publishGlobal(message.cast<double>());
                } else {
                    throw std::runtime_error("Unsupported message type");
                }
            },
            "Publish a message to all subscribers globally", py::arg("message"))
        .def(
            "subscribe",
            [](MessageBus &self, const std::string &name, py::function handler,
               bool async, bool once, py::function filter) {
                if (handler.is_none()) {
                    throw std::runtime_error("Handler function cannot be None");
                }
                if (filter.is_none()) {
                    filter = py::cpp_function(
                        [](const py::object &) { return true; });
                }
                return self.subscribe<std::string>(
                    name,
                    [handler](const std::string &msg) {
                        py::gil_scoped_acquire acquire;
                        handler(msg);
                    },
                    async, once,
                    [filter](const std::string &msg) {
                        py::gil_scoped_acquire acquire;
                        return filter(msg).cast<bool>();
                    });
            },
            "Subscribe to a message", py::arg("name"), py::arg("handler"),
            py::arg("async") = true, py::arg("once") = false,
            py::arg("filter") = py::none())
        .def("unsubscribe", &MessageBus::unsubscribe<std::string>,
             "Unsubscribe from a message using the given token",
             py::arg("token"))
        .def("unsubscribe_all", &MessageBus::unsubscribeAll<std::string>,
             "Unsubscribe all handlers for a given message name or namespace",
             py::arg("name"))
        .def("get_subscriber_count",
             &MessageBus::getSubscriberCount<std::string>,
             "Get the number of subscribers for a given message name or "
             "namespace",
             py::arg("name"))
        .def("has_subscriber", &MessageBus::hasSubscriber<std::string>,
             "Check if there are any subscribers for a given message name or "
             "namespace",
             py::arg("name"))
        .def("clear_all_subscribers", &MessageBus::clearAllSubscribers,
             "Clear all subscribers")
        .def("get_active_namespaces", &MessageBus::getActiveNamespaces,
             "Get the list of active namespaces")
        .def("get_message_history", &MessageBus::getMessageHistory<std::string>,
             "Get the message history for a given message name",
             py::arg("name"),
             py::arg("count") = MessageBus::K_MAX_HISTORY_SIZE);

    bind_message_queue<std::string>(m, "StringMessageQueue");
    bind_message_queue<int>(m, "IntMessageQueue");
    bind_message_queue<double>(m, "DoubleMessageQueue");

    py::class_<ThreadSafeQueue<std::function<void()>>>(m, "ThreadSafeQueue")
        .def(py::init<>())
        .def("push_back", &ThreadSafeQueue<std::function<void()>>::pushBack,
             "Push a task to the back of the queue", py::arg("value"))
        .def("push_front", &ThreadSafeQueue<std::function<void()>>::pushFront,
             "Push a task to the front of the queue", py::arg("value"))
        .def("empty", &ThreadSafeQueue<std::function<void()>>::empty,
             "Check if the queue is empty")
        .def("size", &ThreadSafeQueue<std::function<void()>>::size,
             "Get the size of the queue")
        .def("pop_front", &ThreadSafeQueue<std::function<void()>>::popFront,
             "Pop a task from the front of the queue")
        .def("pop_back", &ThreadSafeQueue<std::function<void()>>::popBack,
             "Pop a task from the back of the queue")
        .def("steal", &ThreadSafeQueue<std::function<void()>>::steal,
             "Steal a task from the back of the queue")
        .def("rotate_to_front",
             &ThreadSafeQueue<std::function<void()>>::rotateToFront,
             "Rotate a task to the front of the queue", py::arg("item"))
        .def("copy_front_and_rotate_to_back",
             &ThreadSafeQueue<std::function<void()>>::copyFrontAndRotateToBack,
             "Copy the front task and rotate it to the back of the queue")
        .def("clear", &ThreadSafeQueue<std::function<void()>>::clear,
             "Clear the queue");

    py::class_<ThreadPool<>>(m, "ThreadPool")
        .def(py::init<unsigned int>(), "Constructor",
             py::arg("number_of_threads") = std::thread::hardware_concurrency())
        .def(
            "enqueue",
            [](ThreadPool<> &self, py::function func) {
                return self.enqueue([func]() {
                    py::gil_scoped_acquire acquire;
                    func();
                });
            },
            "Enqueue a task and return a future")
        .def(
            "enqueue_detach",
            [](ThreadPool<> &self, py::function func) {
                self.enqueueDetach([func]() {
                    py::gil_scoped_acquire acquire;
                    func();
                });
            },
            "Enqueue a task and detach it")
        .def("size", &ThreadPool<>::size,
             "Get the number of threads in the pool")
        .def("wait_for_tasks", &ThreadPool<>::waitForTasks,
             "Wait for all tasks to complete");

    py::class_<TimerTask>(m, "TimerTask")
        .def(py::init<std::function<void()>, unsigned int, int, int>(),
             py::arg("func"), py::arg("delay"), py::arg("repeatCount"),
             py::arg("priority"))
        .def("run", &TimerTask::run)
        .def("getNextExecutionTime", &TimerTask::getNextExecutionTime)
        .def("__lt__", &TimerTask::operator<)
        .def_readwrite("m_func", &TimerTask::m_func)
        .def_readwrite("m_delay", &TimerTask::m_delay)
        .def_readwrite("m_repeatCount", &TimerTask::m_repeatCount)
        .def_readwrite("m_priority", &TimerTask::m_priority)
        .def_readwrite("m_nextExecutionTime", &TimerTask::m_nextExecutionTime);

    py::class_<Timer>(m, "Timer")
        .def(py::init<>())
        .def("setTimeout", &Timer::setTimeout<std::function<void()>>,
             py::arg("func"), py::arg("delay"))
        .def("setInterval", &Timer::setInterval<std::function<void()>>,
             py::arg("func"), py::arg("interval"), py::arg("repeatCount"),
             py::arg("priority"))
        .def("now", &Timer::now)
        .def("cancelAllTasks", &Timer::cancelAllTasks)
        .def("pause", &Timer::pause)
        .def("resume", &Timer::resume)
        .def("stop", &Timer::stop)
        .def("setCallback", &Timer::setCallback<std::function<void()>>,
             py::arg("func"))
        .def("getTaskCount", &Timer::getTaskCount);

    bind_trigger<int>(m, "TriggerInt");
    bind_trigger<std::string>(m, "TriggerString");
    bind_trigger<double>(m, "TriggerDouble");
    bind_trigger<std::function<void()>>(m, "TriggerFunction");

    bind_safe_type<int>(m, "Int");
    bind_safe_type<std::string>(m, "String");
    bind_safe_type<double>(m, "Double");
    bind_safe_type<float>(m, "Float");

    py::class_<RateLimiter::Settings>(m, "RateLimiterSettings")
        .def(py::init<size_t, std::chrono::seconds>(),
             py::arg("max_requests") = 5,
             py::arg("time_window") = std::chrono::seconds(1))
        .def_readwrite("maxRequests", &RateLimiter::Settings::maxRequests)
        .def_readwrite("timeWindow", &RateLimiter::Settings::timeWindow);

    py::class_<RateLimiter>(m, "RateLimiter")
        .def(py::init<>())
        .def("acquire", &RateLimiter::acquire)
        .def("setFunctionLimit", &RateLimiter::setFunctionLimit)
        .def("pause", &RateLimiter::pause)
        .def("resume", &RateLimiter::resume)
        .def("printLog", &RateLimiter::printLog)
        .def("getRejectedRequests", &RateLimiter::getRejectedRequests);

    py::class_<Debounce>(m, "Debounce")
        .def(py::init<std::function<void()>, std::chrono::milliseconds, bool,
                      std::optional<std::chrono::milliseconds>>(),
             py::arg("func"), py::arg("delay"), py::arg("leading") = false,
             py::arg("maxWait") = std::nullopt)
        .def("__call__", &Debounce::operator())
        .def("cancel", &Debounce::cancel)
        .def("flush", &Debounce::flush)
        .def("reset", &Debounce::reset)
        .def("callCount", &Debounce::callCount);

    py::class_<Throttle>(m, "Throttle")
        .def(py::init<std::function<void()>, std::chrono::milliseconds, bool,
                      std::optional<std::chrono::milliseconds>>(),
             py::arg("func"), py::arg("interval"), py::arg("leading") = false,
             py::arg("maxWait") = std::nullopt)
        .def("__call__", &Throttle::operator())
        .def("cancel", &Throttle::cancel)
        .def("reset", &Throttle::reset)
        .def("callCount", &Throttle::callCount);
}