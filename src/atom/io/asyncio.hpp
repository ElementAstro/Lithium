#ifndef ATOM_IO_ASYNC_IO_HPP
#define ATOM_IO_ASYNC_IO_HPP

#ifdef _WIN32
#include <windows.h>
#else
#include <aio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#endif

#ifdef USE_ASIO
#include <asio.hpp>
#endif

#include <coroutine>
#include <string>

namespace atom::io {
struct FileWriter {
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;

    handle_type coro_handle;

    FileWriter(handle_type h);
    ~FileWriter();

    struct promise_type {
        auto get_return_object();
        auto initial_suspend() noexcept;
        auto final_suspend() noexcept;
        void return_void();
        void unhandled_exception();
    };

    bool await_ready() const noexcept;
    void await_suspend(std::coroutine_handle<> h);
    void await_resume() const;
};

FileWriter async_write(const std::string& filename, const std::string& data);
FileWriter async_read(const std::string& filename, std::string& data,
                      std::size_t size);
FileWriter async_delete(const std::string& filename);
FileWriter async_copy(const std::string& src_filename,
                      const std::string& dest_filename);
}  // namespace atom::io

#endif  // ATOM_IO_ASYNC_IO_HPP
