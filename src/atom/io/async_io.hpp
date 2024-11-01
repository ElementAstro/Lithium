#ifndef ATOM_IO_ASYNC_IO_HPP
#define ATOM_IO_ASYNC_IO_HPP

#include <asio.hpp>
#include <filesystem>
#include <functional>
#include <string>
#include <vector>

namespace atom::async::io {

/**
 * @brief Class for performing asynchronous file operations.
 */
class AsyncFile {
public:
    /**
     * @brief Constructs an AsyncFile object.
     * @param io_context The ASIO I/O context.
     */
    explicit AsyncFile(asio::io_context& io_context);

    /**
     * @brief Asynchronously reads the content of a file.
     * @param filename The name of the file to read.
     * @param callback The callback function to call with the file content.
     */
    void asyncRead(const std::string& filename,
                   const std::function<void(const std::string&)>& callback);

    /**
     * @brief Asynchronously writes content to a file.
     * @param filename The name of the file to write to.
     * @param content The content to write to the file.
     * @param callback The callback function to call with the result of the
     * operation.
     */
    void asyncWrite(const std::string& filename, const std::string& content,
                    const std::function<void(bool)>& callback);

    /**
     * @brief Asynchronously deletes a file.
     * @param filename The name of the file to delete.
     * @param callback The callback function to call with the result of the
     * operation.
     */
    void asyncDelete(const std::string& filename,
                     const std::function<void(bool)>& callback);

    /**
     * @brief Asynchronously copies a file.
     * @param src The source file path.
     * @param dest The destination file path.
     * @param callback The callback function to call with the result of the
     * operation.
     */
    void asyncCopy(const std::string& src, const std::string& dest,
                   const std::function<void(bool)>& callback);

    /**
     * @brief Asynchronously reads the content of a file with a timeout.
     * @param filename The name of the file to read.
     * @param timeoutMs The timeout in milliseconds.
     * @param callback The callback function to call with the file content.
     */
    void asyncReadWithTimeout(
        const std::string& filename, int timeoutMs,
        const std::function<void(const std::string&)>& callback);

    /**
     * @brief Asynchronously reads the content of multiple files.
     * @param files The list of files to read.
     * @param callback The callback function to call with the content of the
     * files.
     */
    void asyncBatchRead(
        const std::vector<std::string>& files,
        const std::function<void(const std::vector<std::string>&)>& callback);

    /**
     * @brief Asynchronously retrieves the status of a file.
     * @param filename The name of the file.
     * @param callback The callback function to call with the file status.
     */
    void asyncStat(
        const std::string& filename,
        const std::function<void(bool, std::uintmax_t, std::time_t)>& callback);

    /**
     * @brief Asynchronously moves a file.
     * @param src The source file path.
     * @param dest The destination file path.
     * @param callback The callback function to call with the result of the
     * operation.
     */
    void asyncMove(const std::string& src, const std::string& dest,
                   const std::function<void(bool)>& callback);

    /**
     * @brief Asynchronously changes the permissions of a file.
     * @param filename The name of the file.
     * @param perms The new permissions.
     * @param callback The callback function to call with the result of the
     * operation.
     */
    void asyncChangePermissions(const std::string& filename,
                                std::filesystem::perms perms,
                                const std::function<void(bool)>& callback);

    /**
     * @brief Asynchronously creates a directory.
     * @param path The path of the directory to create.
     * @param callback The callback function to call with the result of the
     * operation.
     */
    void asyncCreateDirectory(const std::string& path,
                              const std::function<void(bool)>& callback);

    /**
     * @brief Asynchronously checks if a file exists.
     * @param filename The name of the file.
     * @param callback The callback function to call with the result of the
     * check.
     */
    void asyncExists(const std::string& filename,
                     const std::function<void(bool)>& callback);

private:
    asio::io_context& io_context_;  ///< The ASIO I/O context.
    asio::steady_timer timer_;      ///< Timer for simulating slow reading.

    static constexpr int kSimulateSlowReadingMs =
        100;  ///< Simulated slow reading time in milliseconds.
};

/**
 * @brief Class for performing asynchronous directory operations.
 */
class AsyncDirectory {
public:
    /**
     * @brief Constructs an AsyncDirectory object.
     * @param io_context The ASIO I/O context.
     */
    explicit AsyncDirectory(asio::io_context& io_context);

    /**
     * @brief Asynchronously creates a directory.
     * @param path The path of the directory to create.
     * @param callback The callback function to call with the result of the
     * operation.
     */
    void asyncCreate(const std::string& path,
                     const std::function<void(bool)>& callback);

    /**
     * @brief Asynchronously removes a directory.
     * @param path The path of the directory to remove.
     * @param callback The callback function to call with the result of the
     * operation.
     */
    void asyncRemove(const std::string& path,
                     const std::function<void(bool)>& callback);

    /**
     * @brief Asynchronously lists the contents of a directory.
     * @param path The path of the directory.
     * @param callback The callback function to call with the list of contents.
     */
    void asyncListContents(
        const std::string& path,
        const std::function<void(std::vector<std::string>)>& callback);

    /**
     * @brief Asynchronously checks if a directory exists.
     * @param path The path of the directory.
     * @param callback The callback function to call with the result of the
     * check.
     */
    void asyncExists(const std::string& path,
                     const std::function<void(bool)>& callback);

private:
    asio::io_context& io_context_;  ///< The ASIO I/O context.
};

}  // namespace atom::async::io

#endif  // ATOM_IO_ASYNC_IO_HPP
