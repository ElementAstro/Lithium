#ifndef LITHIUM_PRELOAD_HPP
#define LITHIUM_PRELOAD_HPP

#include <memory>
#include <string>

#include "atom/type/noncopyable.hpp"

namespace lithium {
/**
 * @class Preloader
 * @brief A class responsible for preloading resources.
 *
 * The Preloader class provides functionality to check, download, and manage
 * resources required by the application. It ensures that all necessary
 * resources are available before they are needed.
 *
 * @note This class is non-copyable.
 */
class Preloader : NonCopyable {
public:
    /**
     * @brief Constructs a new Preloader object.
     */
    Preloader();

    /**
     * @brief Destroys the Preloader object.
     */
    ~Preloader() override;

    /**
     * @brief Checks the availability of resource files.
     *
     * This function verifies if all required resource files are present.
     *
     * @return true if all resource files are available, false otherwise.
     */
    auto checkResources() -> bool;

    /**
     * @brief Downloads missing resource files.
     *
     * This function initiates the download of any resource files that are
     * missing.
     */
    void downloadResources();

    /**
     * @brief Gets the download progress.
     *
     * This function returns the current progress of the resource download
     * process.
     *
     * @return A double value representing the download progress as a percentage
     * (0.0 to 100.0).
     */
    [[nodiscard]] auto getDownloadProgress() const -> double;

    /**
     * @brief Sets the resource server address.
     *
     * This function sets the address of the server from which resources will be
     * downloaded.
     *
     * @param server A string representing the resource server address.
     */
    void setResourceServer(const std::string& server);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};
}  // namespace lithium

#endif  // LITHIUM_PRELOAD_HPP
