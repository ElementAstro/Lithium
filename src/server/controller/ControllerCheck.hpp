#ifndef LITHIUM_SERVER_CONTROLLER_CHECK_HPP
#define LITHIUM_SERVER_CONTROLLER_CHECK_HPP

#include <stdexcept>
#include <string_view>

constexpr auto isAlnum(char character) -> bool {
    return (character >= 'A' && character <= 'Z') ||
           (character >= 'a' && character <= 'z') ||
           (character >= '0' && character <= '9');
}

constexpr auto isValidPathChar(char character) -> bool {
    return isAlnum(character) || character == '_' || character == '-' ||
           character == '.' || character == ':' || character == '@';
}

constexpr auto isWildcard(char character) -> bool { return character == '*'; }

constexpr auto validateParamSegment(std::string_view segment) -> bool {
    if (segment.size() < 3 || segment.front() != '{' || segment.back() != '}') {
        return false;
    }

    if (segment.size() == 3) {
        return false;
    }

    for (size_t index = 1; index < segment.size() - 1; ++index) {
        if (!isValidPathChar(segment[index])) {
            return false;
        }
    }

    return true;
}

constexpr auto validateStaticSegment(std::string_view segment) -> bool {
    if (segment.size() == 1 && isWildcard(segment[0])) {
        return true;
    }

    if (segment == "." || segment == "..") {
        return true;
    }

    if (segment.empty()) {
        return false;
    }
    for (char character : segment) {
        if (!isValidPathChar(character)) {
            return false;
        }
    }
    return true;
}

constexpr auto validatePath(std::string_view path) -> bool {
    if (path.empty() || path.front() != '/' ||
        (path.size() > 1 && path.back() == '/')) {
        return false;
    }

    if (path.size() == 1 && path.front() == '/') {
        return true;
    }

    size_t position = 1;
    while (position <= path.size()) {
        size_t nextPosition = path.find('/', position);
        if (nextPosition == std::string_view::npos) {
            nextPosition = path.size();
        }
        std::string_view segment =
            path.substr(position, nextPosition - position);

        if (segment.empty()) {
            return false;
        }

        if (!validateStaticSegment(segment) && !validateParamSegment(segment)) {
            return false;
        }

        position = nextPosition + 1;
    }

    return true;
}

constexpr auto operator"" _path(const char* str,
                                size_t len) -> const char * {
    std::string_view path(str, len);
    if (!validatePath(path)) {
        throw std::invalid_argument("Invalid path literal");
    }
    return path.data();
}

#endif
