#include "pushd.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <stack>
#include <vector>

#include "atom/log/loguru.hpp"

// Private implementation class
class DirectoryStackImpl {
public:
    std::stack<std::filesystem::path> dirStack;

    [[nodiscard]] auto getStackContents() const
        -> std::vector<std::filesystem::path> {
        std::stack<std::filesystem::path> tempStack = dirStack;
        std::vector<std::filesystem::path> contents;
        while (!tempStack.empty()) {
            contents.push_back(tempStack.top());
            tempStack.pop();
        }
        std::reverse(contents.begin(), contents.end());
        return contents;
    }
};

// DirectoryStack public interface methods

DirectoryStack::DirectoryStack()
    : impl_(std::make_unique<DirectoryStackImpl>()) {}

DirectoryStack::~DirectoryStack() = default;

DirectoryStack::DirectoryStack(const DirectoryStack& other)
    : impl_(std::make_unique<DirectoryStackImpl>(*other.impl_)) {}

auto DirectoryStack::operator=(const DirectoryStack& other) -> DirectoryStack& {
    if (this != &other) {
        impl_ = std::make_unique<DirectoryStackImpl>(*other.impl_);
    }
    return *this;
}

DirectoryStack::DirectoryStack(DirectoryStack&& other) noexcept = default;

auto DirectoryStack::operator=(DirectoryStack&& other) noexcept
    -> DirectoryStack& = default;

void DirectoryStack::pushd(const std::filesystem::path& newDir) {
    try {
        std::filesystem::path currentDir = std::filesystem::current_path();
        impl_->dirStack.push(currentDir);
        std::filesystem::current_path(newDir);
        LOG_F(INFO, "Changed directory to: {}",
              std::filesystem::current_path().string());
    } catch (const std::filesystem::filesystem_error& e) {
        LOG_F(ERROR, "Error: {}", e.what());
    }
}

void DirectoryStack::popd() {
    if (impl_->dirStack.empty()) {
        LOG_F(ERROR, "Directory stack is empty, cannot pop.");
        return;
    }
    try {
        std::filesystem::path previousDir = impl_->dirStack.top();
        impl_->dirStack.pop();
        std::filesystem::current_path(previousDir);
        LOG_F(INFO, "Changed back to directory: {}",
              std::filesystem::current_path().string());
    } catch (const std::filesystem::filesystem_error& e) {
        LOG_F(ERROR, "Error: {}", e.what());
    }
}

void DirectoryStack::peek() const {
    if (impl_->dirStack.empty()) {
        LOG_F(ERROR, "Directory stack is empty.");
        return;
    }
    LOG_F(INFO, "Top directory in stack: {}", impl_->dirStack.top().string());
}

void DirectoryStack::dirs() const {
    LOG_F(INFO, "Current Directory Stack:");
    auto contents = impl_->getStackContents();
    for (size_t i = 0; i < contents.size(); ++i) {
        LOG_F(INFO, "%zu: {}", i, contents[i].string());
    }
}

void DirectoryStack::clear() {
    while (!impl_->dirStack.empty()) {
        impl_->dirStack.pop();
    }
    LOG_F(INFO, "Directory stack cleared.");
}

void DirectoryStack::swap(size_t index1, size_t index2) {
    auto contents = impl_->getStackContents();
    if (index1 >= contents.size() || index2 >= contents.size()) {
        LOG_F(ERROR, "Invalid indices for swap operation.");
        return;
    }
    std::swap(contents[index1], contents[index2]);

    std::stack<std::filesystem::path> newStack;
    for (const auto& dir : contents) {
        newStack.push(dir);
    }
    impl_->dirStack = std::move(newStack);

    LOG_F(INFO, "Swapped directories at indices %zu and %zu.", index1, index2);
    dirs();  // Display the updated stack
}

void DirectoryStack::remove(size_t index) {
    auto contents = impl_->getStackContents();
    if (index >= contents.size()) {
        LOG_F(ERROR, "Invalid index for remove operation.");
        return;
    }
    contents.erase(
        contents.begin() +
        static_cast<std::vector<std::filesystem::path>::difference_type>(
            index));

    std::stack<std::filesystem::path> newStack;
    for (const auto& dir : contents) {
        newStack.push(dir);
    }
    impl_->dirStack = std::move(newStack);

    LOG_F(INFO, "Removed directory at index %zu.", index);
    dirs();  // Display the updated stack
}

void DirectoryStack::gotoIndex(size_t index) {
    auto contents = impl_->getStackContents();
    if (index >= contents.size()) {
        LOG_F(ERROR, "Invalid index for goto operation.");
        return;
    }
    try {
        std::filesystem::current_path(contents[index]);
        LOG_F(INFO, "Changed to directory: {}",
              std::filesystem::current_path().string());
    } catch (const std::filesystem::filesystem_error& e) {
        LOG_F(ERROR, "Error: {}", e.what());
    }
}

void DirectoryStack::saveStackToFile(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file) {
        LOG_F(ERROR, "Error: Unable to open file for writing.");
        return;
    }
    auto contents = impl_->getStackContents();
    for (const auto& dir : contents) {
        file << dir.string() << '\n';
    }
    file.close();
    LOG_F(INFO, "Directory stack saved to {}.", filename);
}

void DirectoryStack::loadStackFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        LOG_F(ERROR, "Error: Unable to open file for reading.");
        return;
    }
    clear();  // Clear the current stack
    std::string line;
    while (std::getline(file, line)) {
        impl_->dirStack.emplace(line);
    }
    file.close();
    LOG_F(INFO, "Directory stack loaded from {}.", filename);
    dirs();  // Display the updated stack
}

auto DirectoryStack::size() const -> size_t { return impl_->dirStack.size(); }

auto DirectoryStack::isEmpty() const -> bool { return impl_->dirStack.empty(); }

void DirectoryStack::showCurrentDirectory() const {
    LOG_F(INFO, "Current Directory: {}",
          std::filesystem::current_path().string());
}
