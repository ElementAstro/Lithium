/*
 * _pybind.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-13

Description: Python Binding of Atom-Async

**************************************************/

#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

#include "async.hpp"
#include "queue.hpp"
#include "thread_wrapper.hpp"
#include "timer.hpp"
#include "trigger.hpp"

namespace py = pybind11;

using namespace Atom::Async;

template <typename Callable, typename... Args>
void start_wrapper(Thread& thread, Callable&& func, Args&&... args) {
    thread.start(std::forward<Callable>(func), std::forward<Args>(args)...);
}

void bind_thread(py::module& m) {
    py::class_<Thread>(m, "Thread")
        .def(py::init<>())
        .def("start", &start_wrapper<py::object, py::args>,
             "Starts a new thread with the specified callable object and "
             "arguments.")
        .def("request_stop", &Thread::request_stop,
             "Requests the thread to stop execution.")
        .def("join", &Thread::join, "Waits for the thread to finish execution.")
        .def("running", &Thread::running,
             "Checks if the thread is currently running.")
        .def("swap", &Thread::swap,
             "Swaps the content of this Thread object with another Thread "
             "object.")
        .def("get_id", &Thread::get_id, "Gets the ID of the thread.")
        .def("get_stop_source", &Thread::get_stop_source,
             "Gets the underlying std::stop_source object.")
        .def("get_stop_token", &Thread::get_stop_token,
             "Gets the underlying std::stop_token object.")
        //.def_property_readonly("thread", &Thread::get_thread,
        //                       "Gets the underlying std::jthread object.")
        .def("__enter__", [](Thread& self) -> Thread& { return self; })
        .def("__exit__", [](Thread& self, py::object, py::object,
                            py::object) { self.join(); })
        .def("__enter__", [](Thread& self) -> Thread& { return self; })
        .def("__exit__", [](Thread& self, py::object, py::object, py::object) {
            self.join();
        });
}

// Bind TimerTask class
void bind_timer_task(py::module& m) {
    py::class_<TimerTask>(m, "TimerTask")
        .def(py::init<std::function<void()>, unsigned int, int, int>())
        .def("__lt__", &TimerTask::operator<)
        .def("run", &TimerTask::run)
        .def("getNextExecutionTime", &TimerTask::getNextExecutionTime);
}

// Bind Timer class
void bind_timer(py::module& m) {
    py::class_<Timer>(m, "Timer")
        .def(py::init<>())
        .def("setTimeout", &Timer::setTimeout<py::function, py::args>,
             "Schedules a task to be executed once after a specified delay.")
        .def("setInterval", &Timer::setInterval<py::function, py::args>,
             "Schedules a task to be executed repeatedly at a specified "
             "interval.")
        .def("now", &Timer::now, "Returns the current time.")
        .def("cancelAllTasks", &Timer::cancelAllTasks,
             "Cancels all scheduled tasks.")
        .def("pause", &Timer::pause, "Pauses the execution of scheduled tasks.")
        .def("resume", &Timer::resume,
             "Resumes the execution of scheduled tasks after pausing.")
        .def("stop", &Timer::stop, "Stops the timer and cancels all tasks.")
        .def("setCallback", &Timer::setCallback<py::function>,
             "Sets a callback function to be called when a task is executed.")
        .def("getTaskCount", &Timer::getTaskCount,
             "Gets the number of scheduled tasks.");
}

/*
// TODO: Fix this
template <typename T>
void bind_thread_safe_queue(py::module& m, const std::string& name) {
    py::class_<ThreadSafeQueue<T>>(m, name.c_str())
        .def(py::init<>())
        .def("put", &ThreadSafeQueue<T>::put)
        .def("take", &ThreadSafeQueue<T>::take)
        .def("destroy", &ThreadSafeQueue<T>::destroy)
        .def("size", &ThreadSafeQueue<T>::size)
        .def("empty", &ThreadSafeQueue<T>::empty)
        .def("clear", &ThreadSafeQueue<T>::clear)
        .def("front", &ThreadSafeQueue<T>::front)
        .def("back", &ThreadSafeQueue<T>::back)
        .def("emplace", &ThreadSafeQueue<T>::template emplace<T>)
        .def("waitFor", &ThreadSafeQueue<T>::template waitFor<T>)
        .def("waitUntilEmpty", &ThreadSafeQueue<T>::waitUntilEmpty)
        .def("extractIf", &ThreadSafeQueue<T>::template extractIf<T>)
        .def("sort", &ThreadSafeQueue<T>::template sort<T>);
}

// Bind AsyncWorker class
template <typename ResultType>
void bind_async_worker(py::module& m, const std::string& name) {
    py::class_<AsyncWorker<ResultType>>(m, name.c_str())
        .def(py::init<>())
        //.def("StartAsync",
        //     &AsyncWorker<ResultType>::template StartAsync<py::function,
        //                                                   py::args>)
        .def("GetResult", &AsyncWorker<ResultType>::GetResult)
        .def("Cancel", &AsyncWorker<ResultType>::Cancel)
        .def("IsDone", &AsyncWorker<ResultType>::IsDone)
        .def("IsActive", &AsyncWorker<ResultType>::IsActive)
        .def("Validate", &AsyncWorker<ResultType>::Validate)
        .def("SetCallback", &AsyncWorker<ResultType>::SetCallback)
        .def("SetTimeout", &AsyncWorker<ResultType>::SetTimeout)
        .def("WaitForCompletion", &AsyncWorker<ResultType>::WaitForCompletion);
}

// Bind AsyncWorkerManager class
template <typename ResultType>
void bind_async_worker_manager(py::module& m, const std::string& name) {
    py::class_<AsyncWorkerManager<ResultType>>(m, name.c_str())
        .def(py::init<>())
        .def(
            "CreateWorker",
            &AsyncWorkerManager<ResultType>::template CreateWorker<py::function,
                                                                   py::args>)
        .def("CancelAll", &AsyncWorkerManager<ResultType>::CancelAll)
        .def("AllDone", &AsyncWorkerManager<ResultType>::AllDone)
        .def("WaitForAll", &AsyncWorkerManager<ResultType>::WaitForAll)
        .def("IsDone", &AsyncWorkerManager<ResultType>::IsDone)
        .def("Cancel", &AsyncWorkerManager<ResultType>::Cancel);
}
*/

PYBIND11_MODULE(atom_async, m) {
    m.doc() = "Atom Async Python Binding";

    // Define the Trigger class and its methods
    py::class_<Trigger<int>>(m, "Trigger")
        .def(py::init<>())
        .def("register_callback", &Trigger<int>::registerCallback)
        .def("unregister_callback", &Trigger<int>::unregisterCallback)
        .def("trigger", &Trigger<int>::trigger)
        .def("schedule_trigger", &Trigger<int>::scheduleTrigger)
        .def("schedule_async_trigger", &Trigger<int>::scheduleAsyncTrigger)
        .def("cancel_trigger", &Trigger<int>::cancelTrigger)
        .def("cancel_all_triggers", &Trigger<int>::cancelAllTriggers);

    

    bind_thread(m);
    bind_timer_task(m);
    bind_timer(m);

    /*
    bind_thread_safe_queue<int>(m, "ThreadSafeQueueInt");
    bind_thread_safe_queue<double>(m, "ThreadSafeQueueDouble");
    bind_thread_safe_queue<std::string>(m, "ThreadSafeQueueString");

    bind_async_worker<int>(m, "AsyncWorkerInt");
    bind_async_worker<float>(m, "AsyncWorkerFloat");
    bind_async_worker<std::string>(m, "AsyncWorkerString");

    bind_async_worker_manager<int>(m, "AsyncWorkerManagerInt");
    bind_async_worker_manager<float>(m, "AsyncWorkerManagerFloat");
    bind_async_worker_manager<std::string>(m, "AsyncWorkerManagerString");
    */
}
