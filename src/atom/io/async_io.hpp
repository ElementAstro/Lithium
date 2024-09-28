#ifndef ATOM_IO_ASYNC_IO_HPP
#define ATOM_IO_ASYNC_IO_HPP

#include <asio.hpp>
#include <filesystem>
#include <functional>
#include <string>
#include <vector>

namespace atom::async::io {
class AsyncFile {
public:
    explicit AsyncFile(asio::io_context& io_context);

    void asyncRead(const std::string& filename,
                   const std::function<void(const std::string&)>& callback);

    void asyncWrite(const std::string& filename, const std::string& content,
                    const std::function<void(bool)>& callback);

    void asyncDelete(const std::string& filename,
                     const std::function<void(bool)>& callback);

    void asyncCopy(const std::string& src, const std::string& dest,
                   const std::function<void(bool)>& callback);

    void asyncReadWithTimeout(
        const std::string& filename, int timeoutMs,
        const std::function<void(const std::string&)>& callback);

    void asyncBatchRead(
        const std::vector<std::string>& files,
        const std::function<void(const std::vector<std::string>&)>& callback);

    void asyncStat(
        const std::string& filename,
        const std::function<void(bool, std::uintmax_t, std::time_t)>& callback);

    void asyncMove(const std::string& src, const std::string& dest,
                   const std::function<void(bool)>& callback);

    void asyncChangePermissions(const std::string& filename,
                                std::filesystem::perms perms,
                                const std::function<void(bool)>& callback);

    void asyncCreateDirectory(const std::string& path,
                              const std::function<void(bool)>& callback);

    void asyncExists(const std::string& filename,
                     const std::function<void(bool)>& callback);

private:
    asio::io_context& io_context_;
    asio::steady_timer timer_;

    static constexpr int kSimulateSlowReadingMs = 100;
};

class AsyncDirectory {
public:
    explicit AsyncDirectory(asio::io_context& io_context);

    void asyncCreate(const std::string& path,
                     const std::function<void(bool)>& callback);

    void asyncRemove(const std::string& path,
                     const std::function<void(bool)>& callback);

    void asyncListContents(
        const std::string& path,
        const std::function<void(std::vector<std::string>)>& callback);

    void asyncExists(const std::string& path,
                     const std::function<void(bool)>& callback);

private:
    asio::io_context& io_context_;
};
}  // namespace atom::async::io

#endif  // ATOM_IO_ASYNC_IO_HPP