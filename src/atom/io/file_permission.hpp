#ifndef ATOM_IO_FILE_PERMISSION_HPP
#define ATOM_IO_FILE_PERMISSION_HPP

#include <optional>
#include <string>

namespace atom::io {
auto compareFileAndSelfPermissions(const std::string &filePath)
    -> std::optional<bool>;
}

#endif