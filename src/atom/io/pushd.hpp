#ifndef ATOM_IO_PUSHD_HPP
#define ATOM_IO_PUSHD_HPP

#include <asio.hpp>
#include <filesystem>
#include <functional>
#include <memory>
#include <string>

namespace atom::io {
// Forward declaration of the implementation class
class DirectoryStackImpl;

/**
 * @class DirectoryStack
 * @brief A class for managing a stack of directory paths, allowing push, pop,
 * and various operations on the stack, with asynchronous support using Asio.
 */
class DirectoryStack {
public:
    /**
     * @brief Constructor
     * @param io_context The Asio io_context to use for asynchronous operations
     */
    explicit DirectoryStack(asio::io_context& io_context);

    /**
     * @brief Destructor
     */
    ~DirectoryStack();

    /**
     * @brief Copy constructor (deleted)
     */
    DirectoryStack(const DirectoryStack& other) = delete;

    /**
     * @brief Copy assignment operator (deleted)
     */
    auto operator=(const DirectoryStack& other) -> DirectoryStack& = delete;

    /**
     * @brief Move constructor
     */
    DirectoryStack(DirectoryStack&& other) noexcept;

    /**
     * @brief Move assignment operator
     */
    auto operator=(DirectoryStack&& other) noexcept -> DirectoryStack&;

    /**
     * @brief Push the current directory onto the stack and change to the
     * specified directory asynchronously.
     * @param new_dir The directory to change to.
     * @param handler The completion handler to be called when the operation
     * completes.
     */
    void asyncPushd(const std::filesystem::path& new_dir,
                    const std::function<void(const std::error_code&)>& handler);

    /**
     * @brief Pop the directory from the stack and change back to it
     * asynchronously.
     * @param handler The completion handler to be called when the operation
     * completes.
     */
    void asyncPopd(const std::function<void(const std::error_code&)>& handler);

    /**
     * @brief View the top directory in the stack without changing to it.
     * @return The top directory in the stack.
     */
    [[nodiscard]] auto peek() const -> std::filesystem::path;

    /**
     * @brief Display the current stack of directories.
     * @return A vector of directory paths in the stack.
     */
    [[nodiscard]] auto dirs() const -> std::vector<std::filesystem::path>;

    /**
     * @brief Clear the directory stack.
     */
    void clear();

    /**
     * @brief Swap two directories in the stack given their indices.
     * @param index1 The first index.
     * @param index2 The second index.
     */
    void swap(size_t index1, size_t index2);

    /**
     * @brief Remove a directory from the stack at the specified index.
     * @param index The index of the directory to remove.
     */
    void remove(size_t index);

    /**
     * @brief Change to the directory at the specified index in the stack
     * asynchronously.
     * @param index The index of the directory to change to.
     * @param handler The completion handler to be called when the operation
     * completes.
     */
    void asyncGotoIndex(
        size_t index,
        const std::function<void(const std::error_code&)>& handler);

    /**
     * @brief Save the directory stack to a file asynchronously.
     * @param filename The name of the file to save the stack to.
     * @param handler The completion handler to be called when the operation
     * completes.
     */
    void asyncSaveStackToFile(
        const std::string& filename,
        const std::function<void(const std::error_code&)>& handler);

    /**
     * @brief Load the directory stack from a file asynchronously.
     * @param filename The name of the file to load the stack from.
     * @param handler The completion handler to be called when the operation
     * completes.
     */
    void asyncLoadStackFromFile(
        const std::string& filename,
        const std::function<void(const std::error_code&)>& handler);

    /**
     * @brief Get the size of the directory stack.
     * @return The number of directories in the stack.
     */
    [[nodiscard]] auto size() const -> size_t;

    /**
     * @brief Check if the directory stack is empty.
     * @return True if the stack is empty, false otherwise.
     */
    [[nodiscard]] auto isEmpty() const -> bool;

    /**
     * @brief Get the current directory path asynchronously.
     * @param handler The completion handler to be called with the current
     * directory path.
     */
    void asyncGetCurrentDirectory(
        const std::function<void(const std::filesystem::path&)>& handler) const;

private:
    std::unique_ptr<DirectoryStackImpl>
        impl_;  ///< Pointer to the implementation.
};
}  // namespace atom::io

#endif  // ATOM_IO_PUSHD_HPP
