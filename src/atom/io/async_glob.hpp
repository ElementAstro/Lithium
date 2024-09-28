#pragma once
#include <asio.hpp>
#include <cassert>
#include <filesystem>
#include <functional>
#include <map>
#include <regex>
#include <string>
#include <vector>

#include "atom/error/exception.hpp"
#include "macro.hpp"

namespace atom::io {

namespace fs = std::filesystem;

class AsyncGlob {
public:
    AsyncGlob(asio::io_context& io_context);
    void glob(const std::string& pathname,
              const std::function<void(std::vector<fs::path>)>& callback,
              bool recursive = false, bool dironly = false);

private:
    void stringReplace(std::string& str, const std::string& from,
                       const std::string& toStr);
    std::string translate(const std::string& pattern);
    std::regex compilePattern(const std::string& pattern);
    bool fnmatch(const fs::path& name, const std::string& pattern);
    std::vector<fs::path> filter(const std::vector<fs::path>& names,
                                 const std::string& pattern);
    fs::path expandTilde(fs::path path);
    bool hasMagic(const std::string& pathname);
    bool isHidden(const std::string& pathname);
    bool isRecursive(const std::string& pattern);
    void iterDirectory(
        const fs::path& dirname, bool dironly,
        const std::function<void(std::vector<fs::path>)>& callback);
    void rlistdir(const fs::path& dirname, bool dironly,
                  const std::function<void(std::vector<fs::path>)>& callback);
    void glob2(const fs::path& dirname, const std::string& pattern,
               bool dironly,
               const std::function<void(std::vector<fs::path>)>& callback);
    void glob1(const fs::path& dirname, const std::string& pattern,
               bool dironly,
               const std::function<void(std::vector<fs::path>)>& callback);
    void glob0(const fs::path& dirname, const fs::path& basename, bool dironly,
               const std::function<void(std::vector<fs::path>)>& callback);

    asio::io_context& io_context_;
};

}  // namespace atom::io
