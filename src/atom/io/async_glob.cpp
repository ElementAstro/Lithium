#include "async_glob.hpp"

namespace atom::io {

AsyncGlob::AsyncGlob(asio::io_context& io_context) : io_context_(io_context) {}

void AsyncGlob::stringReplace(std::string& str, const std::string& from,
                              const std::string& toStr) {
    std::size_t startPos = str.find(from);
    if (startPos == std::string::npos) {
        return;
    }
    str.replace(startPos, from.length(), toStr);
}

auto AsyncGlob::translate(const std::string& pattern) -> std::string {
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
    return std::string{"(("} + resultString + std::string{R"()|[\r\n])$)"};
}

auto AsyncGlob::compilePattern(const std::string& pattern) -> std::regex {
    return std::regex(translate(pattern), std::regex::ECMAScript);
}

auto AsyncGlob::fnmatch(const fs::path& name,
                        const std::string& pattern) -> bool {
    return std::regex_match(name.string(), compilePattern(pattern));
}

auto AsyncGlob::filter(const std::vector<fs::path>& names,
                       const std::string& pattern) -> std::vector<fs::path> {
    std::vector<fs::path> result;
    for (const auto& name : names) {
        if (fnmatch(name, pattern)) {
            result.push_back(name);
        }
    }
    return result;
}

auto AsyncGlob::expandTilde(fs::path path) -> fs::path {
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
        THROW_INVALID_ARGUMENT(
            "error: Unable to expand `~` - HOME environment variable not set.");
    }

    std::string pathStr = path.string();
    if (pathStr[0] == '~') {
        pathStr = std::string(home) + pathStr.substr(1, pathStr.size() - 1);
        return fs::path(pathStr);
    } else {
        return path;
    }
}

auto AsyncGlob::hasMagic(const std::string& pathname) -> bool {
    static const auto MAGIC_CHECK = std::regex("([*?[])");
    return std::regex_search(pathname, MAGIC_CHECK);
}

auto AsyncGlob::isHidden(const std::string& pathname) -> bool {
    return std::regex_match(pathname, std::regex(R"(^(.*\/)*\.[^\.\/]+\/*$)"));
}

auto AsyncGlob::isRecursive(const std::string& pattern) -> bool {
    return pattern == "**";
}

void AsyncGlob::iterDirectory(
    const fs::path& dirname, bool dironly,
    const std::function<void(std::vector<fs::path>)>& callback) {
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
            } catch (std::exception&) {
                // not a directory
                // do nothing
            }
        }

        callback(result);
    });
}

void AsyncGlob::rlistdir(
    const fs::path& dirname, bool dironly,
    const std::function<void(std::vector<fs::path>)>& callback) {
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
    assert(isRecursive(pattern));
    rlistdir(dirname, dironly, callback);
}

void AsyncGlob::glob1(
    const fs::path& dirname, const std::string& pattern, bool dironly,
    const std::function<void(std::vector<fs::path>)>& callback) {
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
}

}  // namespace atom::io
