#include "async_glob.hpp"

#include "atom/log/loguru.hpp"

namespace atom::io {

AsyncGlob::AsyncGlob(asio::io_context& io_context) : io_context_(io_context) {
    LOG_F(INFO, "AsyncGlob constructor called");
}

void AsyncGlob::stringReplace(std::string& str, const std::string& from,
                              const std::string& toStr) {
    LOG_F(INFO, "AsyncGlob::stringReplace called with from: {}, toStr: {}",
          from, toStr);
    std::size_t startPos = str.find(from);
    if (startPos == std::string::npos) {
        LOG_F(WARNING, "Substring not found: {}", from);
        return;
    }
    str.replace(startPos, from.length(), toStr);
    LOG_F(INFO, "String after replacement: {}", str);
}

auto AsyncGlob::translate(const std::string& pattern) -> std::string {
    LOG_F(INFO, "AsyncGlob::translate called with pattern: {}", pattern);
    std::size_t index = 0;
    std::size_t patternSize = pattern.size();
    std::string resultString;

    while (index < patternSize) {
        auto currentChar = pattern[index];
        index += 1;
        if (currentChar == '*') {
            resultString += ".*";
        } else if (currentChar == '?') {
            resultString += ".";
        } else if (currentChar == '[') {
            auto innerIndex = index;
            if (innerIndex < patternSize && pattern[innerIndex] == '!') {
                innerIndex += 1;
            }
            if (innerIndex < patternSize && pattern[innerIndex] == ']') {
                innerIndex += 1;
            }
            while (innerIndex < patternSize && pattern[innerIndex] != ']') {
                innerIndex += 1;
            }
            if (innerIndex >= patternSize) {
                resultString += "\\[";
            } else {
                auto stuff = std::string(
                    pattern.begin() + static_cast<std::ptrdiff_t>(index),
                    pattern.begin() + static_cast<std::ptrdiff_t>(innerIndex));
                if (stuff.find("--") == std::string::npos) {
                    stringReplace(stuff, std::string{"\\"},
                                  std::string{R"(\\)"});
                } else {
                    std::vector<std::string> chunks;
                    std::size_t chunkIndex = 0;
                    if (pattern[index] == '!') {
                        chunkIndex = index + 2;
                    } else {
                        chunkIndex = index + 1;
                    }

                    while (true) {
                        chunkIndex = pattern.find("-", chunkIndex, innerIndex);
                        if (chunkIndex == std::string::npos) {
                            break;
                        }
                        chunks.emplace_back(
                            pattern.begin() +
                                static_cast<std::ptrdiff_t>(index),
                            pattern.begin() +
                                static_cast<std::ptrdiff_t>(chunkIndex));
                        index = chunkIndex + 1;
                        chunkIndex = chunkIndex + 3;
                    }

                    chunks.emplace_back(
                        pattern.begin() + static_cast<std::ptrdiff_t>(index),
                        pattern.begin() +
                            static_cast<std::ptrdiff_t>(innerIndex));
                    bool first = false;
                    for (auto& chunk : chunks) {
                        stringReplace(chunk, std::string{"\\"},
                                      std::string{R"(\\)"});
                        stringReplace(chunk, std::string{"-"},
                                      std::string{R"(\-)"});
                        if (first) {
                            stuff += chunk;
                            first = false;
                        } else {
                            stuff += "-" + chunk;
                        }
                    }
                }

                std::string result;
                std::regex_replace(std::back_inserter(result), stuff.begin(),
                                   stuff.end(),
                                   std::regex(std::string{R"([&~|])"}),
                                   std::string{R"(\\\1)"});
                stuff = result;
                index = innerIndex + 1;
                if (stuff[0] == '!') {
                    stuff = "^" + std::string(stuff.begin() + 1, stuff.end());
                } else if (stuff[0] == '^' || stuff[0] == '[') {
                    stuff = "\\\\" + stuff;
                }
                resultString += "[" + stuff + "]";
            }
        } else {
            static std::string specialCharacters =
                "()[]{}?*+-|^$\\.&~# \t\n\r\v\f";
            static std::map<int, std::string> specialCharactersMap;
            if (specialCharactersMap.empty()) {
                for (auto& specialChar : specialCharacters) {
                    specialCharactersMap.insert(std::make_pair(
                        static_cast<int>(specialChar),
                        std::string{"\\"} + std::string(1, specialChar)));
                }
            }
            if (specialCharacters.find(currentChar) != std::string::npos) {
                resultString +=
                    specialCharactersMap[static_cast<int>(currentChar)];
            } else {
                resultString += currentChar;
            }
        }
    }
    LOG_F(INFO, "Translated pattern: {}", resultString);
    return std::string{"(("} + resultString + std::string{R"()|[\r\n])$)"};
}

auto AsyncGlob::compilePattern(const std::string& pattern) -> std::regex {
    LOG_F(INFO, "AsyncGlob::compilePattern called with pattern: {}", pattern);
    return std::regex(translate(pattern), std::regex::ECMAScript);
}

auto AsyncGlob::fnmatch(const fs::path& name,
                        const std::string& pattern) -> bool {
    LOG_F(INFO, "AsyncGlob::fnmatch called with name: {}, pattern: {}",
          name.string(), pattern);
    bool result = std::regex_match(name.string(), compilePattern(pattern));
    LOG_F(INFO, "AsyncGlob::fnmatch returning: {}", result);
    return result;
}

auto AsyncGlob::filter(const std::vector<fs::path>& names,
                       const std::string& pattern) -> std::vector<fs::path> {
    LOG_F(INFO, "AsyncGlob::filter called with pattern: {}", pattern);
    std::vector<fs::path> result;
    for (const auto& name : names) {
        if (fnmatch(name, pattern)) {
            result.push_back(name);
        }
    }
    LOG_F(INFO, "AsyncGlob::filter returning {} paths", result.size());
    return result;
}

auto AsyncGlob::expandTilde(fs::path path) -> fs::path {
    LOG_F(INFO, "AsyncGlob::expandTilde called with path: {}", path.string());
    if (path.empty()) {
        return path;
    }

#ifdef _WIN32
    const char* homeVariable = "USERNAME";
#else
    const char* homeVariable = "USER";
#endif
    const char* home = getenv(homeVariable);
    if (!home) {
        LOG_F(ERROR,
              "Unable to expand `~` - HOME environment variable not set.");
        THROW_INVALID_ARGUMENT(
            "error: Unable to expand `~` - HOME environment variable not set.");
    }

    std::string pathStr = path.string();
    if (pathStr[0] == '~') {
        pathStr = std::string(home) + pathStr.substr(1, pathStr.size() - 1);
        fs::path expandedPath(pathStr);
        LOG_F(INFO, "Expanded path: {}", expandedPath.string());
        return expandedPath;
    } else {
        return path;
    }
}

auto AsyncGlob::hasMagic(const std::string& pathname) -> bool {
    LOG_F(INFO, "AsyncGlob::hasMagic called with pathname: {}", pathname);
    static const auto MAGIC_CHECK = std::regex("([*?[])");
    bool result = std::regex_search(pathname, MAGIC_CHECK);
    LOG_F(INFO, "AsyncGlob::hasMagic returning: {}", result);
    return result;
}

auto AsyncGlob::isHidden(const std::string& pathname) -> bool {
    LOG_F(INFO, "AsyncGlob::isHidden called with pathname: {}", pathname);
    bool result =
        std::regex_match(pathname, std::regex(R"(^(.*\/)*\.[^\.\/]+\/*$)"));
    LOG_F(INFO, "AsyncGlob::isHidden returning: {}", result);
    return result;
}

auto AsyncGlob::isRecursive(const std::string& pattern) -> bool {
    LOG_F(INFO, "AsyncGlob::isRecursive called with pattern: {}", pattern);
    bool result = pattern == "**";
    LOG_F(INFO, "AsyncGlob::isRecursive returning: {}", result);
    return result;
}

void AsyncGlob::iterDirectory(
    const fs::path& dirname, bool dironly,
    const std::function<void(std::vector<fs::path>)>& callback) {
    LOG_F(INFO, "AsyncGlob::iterDirectory called with dirname: {}, dironly: {}",
          dirname.string(), dironly);
    io_context_.post([dirname, dironly, callback]() {
        std::vector<fs::path> result;
        auto currentDirectory = dirname;
        if (currentDirectory.empty()) {
            currentDirectory = fs::current_path();
        }

        if (fs::exists(currentDirectory)) {
            try {
                for (const auto& entry : fs::directory_iterator(
                         currentDirectory,
                         fs::directory_options::follow_directory_symlink |
                             fs::directory_options::skip_permission_denied)) {
                    if (!dironly || entry.is_directory()) {
                        if (dirname.is_absolute()) {
                            result.push_back(entry.path());
                        } else {
                            result.push_back(fs::relative(entry.path()));
                        }
                    }
                }
            } catch (std::exception& e) {
                LOG_F(ERROR, "Exception in iterDirectory: {}", e.what());
            }
        }

        callback(result);
    });
}

void AsyncGlob::rlistdir(
    const fs::path& dirname, bool dironly,
    const std::function<void(std::vector<fs::path>)>& callback) {
    LOG_F(INFO, "AsyncGlob::rlistdir called with dirname: {}, dironly: {}",
          dirname.string(), dironly);
    iterDirectory(
        dirname, dironly,
        [this, dironly, callback](std::vector<fs::path> names) {
            std::vector<fs::path> result;
            for (auto& name : names) {
                if (!isHidden(name.string())) {
                    result.push_back(name);
                    rlistdir(name, dironly,
                             [&result](std::vector<fs::path> subNames) {
                                 result.insert(result.end(), subNames.begin(),
                                               subNames.end());
                             });
                }
            }
            callback(result);
        });
}

void AsyncGlob::glob2(
    const fs::path& dirname, const std::string& pattern, bool dironly,
    const std::function<void(std::vector<fs::path>)>& callback) {
    LOG_F(INFO,
          "AsyncGlob::glob2 called with dirname: {}, pattern: {}, dironly: {}",
          dirname.string(), pattern, dironly);
    assert(isRecursive(pattern));
    rlistdir(dirname, dironly, callback);
}

void AsyncGlob::glob1(
    const fs::path& dirname, const std::string& pattern, bool dironly,
    const std::function<void(std::vector<fs::path>)>& callback) {
    LOG_F(INFO,
          "AsyncGlob::glob1 called with dirname: {}, pattern: {}, dironly: {}",
          dirname.string(), pattern, dironly);
    iterDirectory(dirname, dironly,
                  [this, pattern, callback](std::vector<fs::path> names) {
                      std::vector<fs::path> filteredNames;
                      for (auto& name : names) {
                          if (!isHidden(name.string())) {
                              filteredNames.push_back(name.filename());
                          }
                      }
                      callback(filter(filteredNames, pattern));
                  });
}

void AsyncGlob::glob0(
    const fs::path& dirname, const fs::path& basename, bool dironly,
    const std::function<void(std::vector<fs::path>)>& callback) {
    LOG_F(INFO,
          "AsyncGlob::glob0 called with dirname: {}, basename: {}, dironly: {}",
          dirname.string(), basename.string(), dironly);
    io_context_.post([dirname, basename, dironly, callback]() {
        std::vector<fs::path> result;
        if (basename.empty()) {
            if (fs::is_directory(dirname)) {
                result = {basename};
            }
        } else {
            if (fs::exists(dirname / basename)) {
                result = {basename};
            }
        }
        callback(result);
    });
}

void AsyncGlob::glob(const std::string& pathname,
                     const std::function<void(std::vector<fs::path>)>& callback,
                     bool recursive, bool dironly) {
    LOG_F(
        INFO,
        "AsyncGlob::glob called with pathname: {}, recursive: {}, dironly: {}",
        pathname, recursive, dironly);
    auto path = fs::path(pathname);

    if (pathname[0] == '~') {
        path = expandTilde(path);
    }

    auto dirname = path.parent_path();
    const auto BASENAME = path.filename();

    if (!hasMagic(pathname)) {
        assert(!dironly);
        if (!BASENAME.empty()) {
            if (fs::exists(path)) {
                callback({path});
            } else {
                callback({});
            }
        } else {
            if (fs::is_directory(dirname)) {
                callback({path});
            } else {
                callback({});
            }
        }
        return;
    }

    std::function<void(const fs::path&, const std::string&, bool,
                       const std::function<void(std::vector<fs::path>)>&)>
        globInDir;
    if (hasMagic(BASENAME.string())) {
        if (recursive && isRecursive(BASENAME.string())) {
            globInDir =
                [this](const fs::path& dir, const std::string& pattern,
                       bool dironly,
                       const std::function<void(std::vector<fs::path>)>& cb) {
                    glob2(dir, pattern, dironly, cb);
                };
        } else {
            globInDir =
                [this](const fs::path& dir, const std::string& pattern,
                       bool dironly,
                       const std::function<void(std::vector<fs::path>)>& cb) {
                    glob1(dir, pattern, dironly, cb);
                };
        }
    } else {
        globInDir = [this](
                        const fs::path& dir, const std::string& pattern,
                        bool dironly,
                        const std::function<void(std::vector<fs::path>)>& cb) {
            glob0(dir, pattern, dironly, cb);
        };
    }

    if (dirname != fs::path(pathname) && hasMagic(dirname.string())) {
        glob(
            dirname.string(),
            [this, BASENAME, dironly, globInDir,
             callback](std::vector<fs::path> dirs) {
                std::vector<fs::path> result;
                for (auto& dir : dirs) {
                    globInDir(dir, BASENAME.string(), dironly,
                              [&result](std::vector<fs::path> names) {
                                  result.insert(result.end(), names.begin(),
                                                names.end());
                              });
                }
                callback(result);
            },
            recursive, true);
    } else {
        std::vector<fs::path> dirs = {dirname};
        std::vector<fs::path> result;
        for (auto& dir : dirs) {
            globInDir(dir, BASENAME.string(), dironly,
                      [&result](std::vector<fs::path> names) {
                          result.insert(result.end(), names.begin(),
                                        names.end());
                      });
        }
        callback(result);
    }
    LOG_F(INFO, "AsyncGlob::glob completed");
}

}  // namespace atom::io