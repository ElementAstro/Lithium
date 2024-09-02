#include "pushd.hpp"
#include <filesystem>
#include <stack>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>

// Private implementation class
class DirectoryStackImpl {
public:
    std::stack<std::filesystem::path> dir_stack;

    std::vector<std::filesystem::path> get_stack_contents() const {
        std::stack<std::filesystem::path> temp_stack = dir_stack;
        std::vector<std::filesystem::path> contents;
        while (!temp_stack.empty()) {
            contents.push_back(temp_stack.top());
            temp_stack.pop();
        }
        std::reverse(contents.begin(), contents.end());
        return contents;
    }
};

// DirectoryStack public interface methods

DirectoryStack::DirectoryStack() : impl(new DirectoryStackImpl) {}

DirectoryStack::~DirectoryStack() {
    delete impl;
}

void DirectoryStack::pushd(const std::filesystem::path& new_dir) {
    try {
        std::filesystem::path current_dir = std::filesystem::current_path();
        impl->dir_stack.push(current_dir);
        std::filesystem::current_path(new_dir);
        std::cout << "Changed directory to: " << std::filesystem::current_path() << '\n';
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error: " << e.what() << '\n';
    }
}

void DirectoryStack::popd() {
    if (impl->dir_stack.empty()) {
        std::cerr << "Directory stack is empty, cannot pop.\n";
        return;
    }
    try {
        std::filesystem::path previous_dir = impl->dir_stack.top();
        impl->dir_stack.pop();
        std::filesystem::current_path(previous_dir);
        std::cout << "Changed back to directory: " << std::filesystem::current_path() << '\n';
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error: " << e.what() << '\n';
    }
}

void DirectoryStack::peek() const {
    if (impl->dir_stack.empty()) {
        std::cerr << "Directory stack is empty.\n";
        return;
    }
    std::cout << "Top directory in stack: " << impl->dir_stack.top() << '\n';
}

void DirectoryStack::dirs() const {
    std::cout << "Current Directory Stack:\n";
    auto contents = impl->get_stack_contents();
    for (size_t i = 0; i < contents.size(); ++i) {
        std::cout << i << ": " << contents[i] << '\n';
    }
}

void DirectoryStack::clear() {
    while (!impl->dir_stack.empty()) {
        impl->dir_stack.pop();
    }
    std::cout << "Directory stack cleared.\n";
}

void DirectoryStack::swap(size_t index1, size_t index2) {
    auto contents = impl->get_stack_contents();
    if (index1 >= contents.size() || index2 >= contents.size()) {
        std::cerr << "Invalid indices for swap operation.\n";
        return;
    }
    std::swap(contents[index1], contents[index2]);

    std::stack<std::filesystem::path> new_stack;
    for (auto it = contents.rbegin(); it != contents.rend(); ++it) {
        new_stack.push(*it);
    }
    impl->dir_stack = new_stack;

    std::cout << "Swapped directories at indices " << index1 << " and " << index2 << ".\n";
    dirs();  // Display the updated stack
}

void DirectoryStack::remove(size_t index) {
    auto contents = impl->get_stack_contents();
    if (index >= contents.size()) {
        std::cerr << "Invalid index for remove operation.\n";
        return;
    }
    contents.erase(contents.begin() + index);

    std::stack<std::filesystem::path> new_stack;
    for (auto it = contents.rbegin(); it != contents.rend(); ++it) {
        new_stack.push(*it);
    }
    impl->dir_stack = new_stack;

    std::cout << "Removed directory at index " << index << ".\n";
    dirs();  // Display the updated stack
}

void DirectoryStack::goto_index(size_t index) {
    auto contents = impl->get_stack_contents();
    if (index >= contents.size()) {
        std::cerr << "Invalid index for goto operation.\n";
        return;
    }
    try {
        std::filesystem::current_path(contents[index]);
        std::cout << "Changed to directory: " << std::filesystem::current_path() << '\n';
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error: " << e.what() << '\n';
    }
}

void DirectoryStack::save_stack_to_file(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file) {
        std::cerr << "Error: Unable to open file for writing.\n";
        return;
    }
    auto contents = impl->get_stack_contents();
    for (const auto& dir : contents) {
        file << dir.string() << '\n';
    }
    file.close();
    std::cout << "Directory stack saved to " << filename << ".\n";
}

void DirectoryStack::load_stack_from_file(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Error: Unable to open file for reading.\n";
        return;
    }
    clear();  // Clear the current stack
    std::string line;
    while (std::getline(file, line)) {
        impl->dir_stack.push(line);
    }
    file.close();
    std::cout << "Directory stack loaded from " << filename << ".\n";
    dirs();  // Display the updated stack
}

size_t DirectoryStack::size() const {
    return impl->dir_stack.size();
}

bool DirectoryStack::is_empty() const {
    return impl->dir_stack.empty();
}

void DirectoryStack::show_current_directory() const {
    std::cout << "Current Directory: " << std::filesystem::current_path() << '\n';
}
