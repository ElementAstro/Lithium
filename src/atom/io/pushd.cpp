#include "pushd.hpp"

#include <algorithm>
#include <fstream>
#include <stack>

#include "atom/log/loguru.hpp"

namespace atom::io {
class DirectoryStackImpl {
public:
    explicit DirectoryStackImpl(asio::io_context& io_context)
        : io_context_(io_context), strand_(io_context) {}

    void asyncPushd(
        const std::filesystem::path& new_dir,
        const std::function<void(const std::error_code&)>& handler) {
        LOG_F(INFO, "asyncPushd called with new_dir: %s",
              new_dir.string().c_str());
        asio::post(strand_, [this, new_dir, handler]() {
            std::error_code errorCode;
            std::filesystem::path currentDir =
                std::filesystem::current_path(errorCode);
            if (!errorCode) {
                dirStack_.push(currentDir);
                std::filesystem::current_path(new_dir, errorCode);
            }
            LOG_F(INFO, "asyncPushd completed with error code: %d",
                  errorCode.value());
            handler(errorCode);
        });
    }

    void asyncPopd(const std::function<void(const std::error_code&)>& handler) {
        LOG_F(INFO, "asyncPopd called");
        asio::post(strand_, [this, handler]() {
            std::error_code errorCode;
            if (!dirStack_.empty()) {
                std::filesystem::path prevDir = dirStack_.top();
                dirStack_.pop();
                std::filesystem::current_path(prevDir, errorCode);
            } else {
                errorCode = std::make_error_code(std::errc::invalid_argument);
            }
            LOG_F(INFO, "asyncPopd completed with error code: %d",
                  errorCode.value());
            handler(errorCode);
        });
    }

    [[nodiscard]] auto getStackContents() const
        -> std::vector<std::filesystem::path> {
        std::stack<std::filesystem::path> tempStack = dirStack_;
        std::vector<std::filesystem::path> contents;
        while (!tempStack.empty()) {
            contents.push_back(tempStack.top());
            tempStack.pop();
        }
        std::reverse(contents.begin(), contents.end());
        return contents;
    }

    void asyncGotoIndex(
        size_t index,
        const std::function<void(const std::error_code&)>& handler) {
        LOG_F(INFO, "asyncGotoIndex called with index: %zu", index);
        asio::post(strand_, [this, index, handler]() {
            std::error_code errorCode;
            auto contents = getStackContents();
            if (index < contents.size()) {
                std::filesystem::current_path(contents[index], errorCode);
            } else {
                errorCode = std::make_error_code(std::errc::invalid_argument);
            }
            LOG_F(INFO, "asyncGotoIndex completed with error code: %d",
                  errorCode.value());
            handler(errorCode);
        });
    }

    void asyncSaveStackToFile(
        const std::string& filename,
        const std::function<void(const std::error_code&)>& handler) {
        LOG_F(INFO, "asyncSaveStackToFile called with filename: %s",
              filename.c_str());
        asio::post(strand_, [this, filename, handler]() {
            std::error_code errorCode;
            std::ofstream file(filename);
            if (file) {
                auto contents = getStackContents();
                for (const auto& dir : contents) {
                    file << dir.string() << '\n';
                }
            } else {
                errorCode = std::make_error_code(std::errc::io_error);
            }
            LOG_F(INFO, "asyncSaveStackToFile completed with error code: %d",
                  errorCode.value());
            handler(errorCode);
        });
    }

    void asyncLoadStackFromFile(
        const std::string& filename,
        const std::function<void(const std::error_code&)>& handler) {
        LOG_F(INFO, "asyncLoadStackFromFile called with filename: %s",
              filename.c_str());
        asio::post(strand_, [this, filename, handler]() {
            std::error_code errorCode;
            std::ifstream file(filename);
            if (file) {
                std::stack<std::filesystem::path> newStack;
                std::string line;
                while (std::getline(file, line)) {
                    newStack.emplace(line);
                }
                dirStack_ = std::move(newStack);
            } else {
                errorCode = std::make_error_code(std::errc::io_error);
            }
            LOG_F(INFO, "asyncLoadStackFromFile completed with error code: %d",
                  errorCode.value());
            handler(errorCode);
        });
    }

    void asyncGetCurrentDirectory(
        const std::function<void(const std::filesystem::path&)>& handler)
        const {
        LOG_F(INFO, "asyncGetCurrentDirectory called");
        asio::post(strand_, [handler]() {
            auto currentPath = std::filesystem::current_path();
            LOG_F(INFO,
                  "asyncGetCurrentDirectory completed with current path: %s",
                  currentPath.string().c_str());
            handler(currentPath);
        });
    }

    std::stack<std::filesystem::path> dirStack_;
    asio::io_context::strand strand_;
    asio::io_context& io_context_;
};

// DirectoryStack public interface methods implementation

DirectoryStack::DirectoryStack(asio::io_context& io_context)
    : impl_(std::make_unique<DirectoryStackImpl>(io_context)) {}

DirectoryStack::~DirectoryStack() = default;

DirectoryStack::DirectoryStack(DirectoryStack&& other) noexcept = default;

auto DirectoryStack::operator=(DirectoryStack&& other) noexcept
    -> DirectoryStack& = default;

void DirectoryStack::asyncPushd(
    const std::filesystem::path& new_dir,
    const std::function<void(const std::error_code&)>& handler) {
    impl_->asyncPushd(new_dir, handler);
}

void DirectoryStack::asyncPopd(
    const std::function<void(const std::error_code&)>& handler) {
    impl_->asyncPopd(handler);
}

auto DirectoryStack::peek() const -> std::filesystem::path {
    return impl_->dirStack_.empty() ? std::filesystem::path()
                                    : impl_->dirStack_.top();
}

auto DirectoryStack::dirs() const -> std::vector<std::filesystem::path> {
    return impl_->getStackContents();
}

void DirectoryStack::clear() {
    impl_->dirStack_ = std::stack<std::filesystem::path>();
}

void DirectoryStack::swap(size_t index1, size_t index2) {
    auto contents = impl_->getStackContents();
    if (index1 < contents.size() && index2 < contents.size()) {
        std::swap(contents[index1], contents[index2]);
        impl_->dirStack_ =
            std::stack<std::filesystem::path>(std::deque<std::filesystem::path>(
                contents.begin(), contents.end()));
    }
}

void DirectoryStack::remove(size_t index) {
    auto contents = impl_->getStackContents();
    if (index < contents.size()) {
        contents.erase(contents.begin() + static_cast<long>(index));
        impl_->dirStack_ =
            std::stack<std::filesystem::path>(std::deque<std::filesystem::path>(
                contents.begin(), contents.end()));
    }
}

void DirectoryStack::asyncGotoIndex(
    size_t index, const std::function<void(const std::error_code&)>& handler) {
    impl_->asyncGotoIndex(index, handler);
}

void DirectoryStack::asyncSaveStackToFile(
    const std::string& filename,
    const std::function<void(const std::error_code&)>& handler) {
    impl_->asyncSaveStackToFile(filename, handler);
}

void DirectoryStack::asyncLoadStackFromFile(
    const std::string& filename,
    const std::function<void(const std::error_code&)>& handler) {
    impl_->asyncLoadStackFromFile(filename, handler);
}

auto DirectoryStack::size() const -> size_t { return impl_->dirStack_.size(); }

auto DirectoryStack::isEmpty() const -> bool {
    return impl_->dirStack_.empty();
}

void DirectoryStack::asyncGetCurrentDirectory(
    const std::function<void(const std::filesystem::path&)>& handler) const {
    impl_->asyncGetCurrentDirectory(handler);
}

}  // namespace atom::io