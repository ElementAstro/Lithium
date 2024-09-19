#ifndef DIRECTORYSTACK_H
#define DIRECTORYSTACK_H

#include <filesystem>
#include <memory>
#include <string>

// Forward declaration of the implementation class
class DirectoryStackImpl;

/**
 * @class DirectoryStack
 * @brief A class for managing a stack of directory paths, allowing push, pop,
 * and various operations on the stack.
 */
class DirectoryStack {
public:
    /**
     * @brief Constructor
     */
    DirectoryStack();

    /**
     * @brief Destructor
     */
    ~DirectoryStack();

    /**
     * @brief Copy constructor
     */
    DirectoryStack(const DirectoryStack& other);

    /**
     * @brief Copy assignment operator
     */
    auto operator=(const DirectoryStack& other) -> DirectoryStack&;

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
     * specified directory.
     * @param new_dir The directory to change to.
     */
    void pushd(const std::filesystem::path& new_dir);

    /**
     * @brief Pop the directory from the stack and change back to it.
     */
    void popd();

    /**
     * @brief View the top directory in the stack without changing to it.
     */
    void peek() const;

    /**
     * @brief Display the current stack of directories.
     */
    void dirs() const;

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
     * @brief Change to the directory at the specified index in the stack.
     * @param index The index of the directory to change to.
     */
    void gotoIndex(size_t index);

    /**
     * @brief Save the directory stack to a file.
     * @param filename The name of the file to save the stack to.
     */
    void saveStackToFile(const std::string& filename) const;

    /**
     * @brief Load the directory stack from a file.
     * @param filename The name of the file to load the stack from.
     */
    void loadStackFromFile(const std::string& filename);

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
     * @brief Show the current directory path.
     */
    void showCurrentDirectory() const;

private:
    std::unique_ptr<DirectoryStackImpl>
        impl_;  ///< Pointer to the implementation.
};

#endif  // DIRECTORYSTACK_H
