#include "atom/components/component.hpp"
#include "atom/components/registry.hpp"

#include "atom/function/overload.hpp"

#include "atom/io/compress.hpp"
#include "atom/io/glob.hpp"
#include "atom/io/io.hpp"

#include "atom/log/loguru.hpp"

using namespace atom::io;

ATOM_MODULE(atom_io, [](Component &component) {
    DLOG_F(INFO, "Loading module {}", component.getName());

    component.def("compress", &compressFile, "compression", "Compress a file");
    component.def("decompress", &decompressFile, "compression",
                  "Decompress a file");
    component.def("create_zip", &createZip, "compression", "Create a zip file");
    component.def("extract_zip", &extractZip, "compression",
                  "Extract a zip file");
    component.def("compress_folder", &compressFolder, "compression",
                  "Compress a folder");

    component.def("translate", &translate, "pattern_matching",
                  "Translate a pattern");
    component.def("compile_pattern", &compilePattern, "pattern_matching",
                  "Compile a pattern");
    component.def("fnmatch", &fnmatch, "pattern_matching",
                  "Check if a name matches a pattern");
    component.def("filter", &filter, "pattern_matching",
                  "Filter a list of names");
    component.def("expand_tilde", &expandTilde, "path_operations",
                  "Expand a tilde");
    component.def("has_magic", &hasMagic, "pattern_matching",
                  "Check if a pattern has magic");
    component.def("is_hidden", &isHidden, "path_operations",
                  "Check if a path is hidden");
    component.def("is_recursive", &isRecursive, "pattern_matching",
                  "Check if a pattern is recursive");
    component.def("iter_dir", &iterDirectory, "directory_operations",
                  "Iterate a directory");
    component.def("rlistdir", &rlistdir, "directory_operations",
                  "Recursively list a directory");
    component.def("glob_s",
                  atom::meta::overload_cast<const std::string &>(glob),
                  "pattern_matching", "Glob a list of files");
    component.def(
        "glob_v",
        atom::meta::overload_cast<const std::vector<std::string> &>(glob),
        "pattern_matching", "Glob a list of files");
    component.def<const std::string &>("rglob", &rglob, "pattern_matching",
                                       "Recursively glob a list of files");
    component.def("glob0", &glob0, "pattern_matching", "Glob0 a list of files");
    component.def("glob1", &glob1, "pattern_matching", "Glob1 a list of files");
    component.def("glob2", &glob2, "pattern_matching", "Glob2 a list of files");

    component.def(
        "mkdir",
        [](const std::string &path) -> bool { return createDirectory(path); },
        "directory_operations", "Create a directory");
    component.def("mkdir_r", &createDirectoriesRecursive,
                  "directory_operations", "Create a directory recursively");
    component.def("rmdir", &removeDirectory, "directory_operations",
                  "Remove a directory");
    component.def("rmdir_r", &removeDirectoriesRecursive,
                  "directory_operations", "Remove a directory recursively");
    component.def("move", &moveDirectory, "directory_operations",
                  "Move a directory");
    component.def("rename", &renameDirectory, "directory_operations",
                  "Rename a directory");
    component.def("copy", &copyFile, "file_operations", "Copy a file");
    component.def("move_file", &moveFile, "file_operations", "Move a file");
    component.def("rename_file", &renameFile, "file_operations",
                  "Rename a file");
    component.def("remove", &removeFile, "file_operations", "Remove a file");
    component.def("mksymlink", &createSymlink, "file_operations",
                  "Create a symbolic link");
    component.def("rmsymlink", &removeSymlink, "file_operations",
                  "Remove a symbolic link");

    DLOG_F(INFO, "Loaded module {}", component.getName());
});
