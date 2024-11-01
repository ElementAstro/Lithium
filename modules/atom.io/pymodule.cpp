#include <pybind11/pybind11.h>

#include "atom/io/compress.hpp"
#include "atom/io/glob.hpp"
#include "atom/io/io.hpp"

namespace py = pybind11;
using namespace atom::io;

PYBIND11_MODULE(atom_io, m) {
    m.def("compress", &compressFile, "Compress a file");
    m.def("decompress", &decompressFile, "Decompress a file");
    m.def("create_zip", &createZip, "Create a zip file");
    m.def("extract_zip", &extractZip, "Extract a zip file");
    m.def("compress_folder", &compressFolder, "Compress a folder");

    m.def("translate", &translate, "Translate a pattern");
    m.def("compile_pattern", &compilePattern, "Compile a pattern");
    m.def("fnmatch", &fnmatch, "Check if a name matches a pattern");
    m.def("filter", &filter, "Filter a list of names");
    m.def("expand_tilde", &expandTilde, "Expand a tilde");
    m.def("has_magic", &hasMagic, "Check if a pattern has magic");
    m.def("is_hidden", &isHidden, "Check if a path is hidden");
    m.def("is_recursive", &isRecursive, "Check if a pattern is recursive");
    m.def("iter_dir", &iterDirectory, "Iterate a directory");
    m.def("rlistdir", &rlistdir, "Recursively list a directory");
    m.def("glob_s", atom::meta::overload_cast<const std::string &>(glob),
          "Glob a list of files");
    m.def("glob_v",
          atom::meta::overload_cast<const std::vector<std::string> &>(glob),
          "Glob a list of files");
    m.def("rglob", &rglob, "Recursively glob a list of files");
    m.def("glob0", &glob0, "Glob0 a list of files");
    m.def("glob1", &glob1, "Glob1 a list of files");
    m.def("glob2", &glob2, "Glob2 a list of files");

    m.def(
        "mkdir",
        [](const std::string &path) -> bool { return createDirectory(path); },
        "Create a directory");
    m.def("mkdir_r", &createDirectoriesRecursive,
          "Create a directory recursively");
    m.def("rmdir", &removeDirectory, "Remove a directory");
    m.def("rmdir_r", &removeDirectoriesRecursive,
          "Remove a directory recursively");
    m.def("move", &moveDirectory, "Move a directory");
    m.def("rename", &renameDirectory, "Rename a directory");
    m.def("copy", &copyFile, "Copy a file");
    m.def("move_file", &moveFile, "Move a file");
    m.def("rename_file", &renameFile, "Rename a file");
    m.def("remove", &removeFile, "Remove a file");
    m.def("mksymlink", &createSymlink, "Create a symbolic link");
    m.def("rmsymlink", &removeSymlink, "Remove a symbolic link");
}
