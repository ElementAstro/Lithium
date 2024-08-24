#include "atom/components/component.hpp"
#include "atom/components/registry.hpp"

#include "atom/io/compress.hpp"
#include "atom/io/glob.hpp"
#include "atom/io/io.hpp"

#include "atom/log/loguru.hpp"

using namespace atom::io;

ATOM_MODULE(atom_io, [](Component &component) {
    DLOG_F(INFO, "Loading module {}", component.getName());

    component.def("compress", &compressFile, "Compress a file");
    component.def("decompress", &decompressFile, "Decompress a file");
    component.def("create_zip", &createZip, "Create a zip file");
    component.def("extract_zip", &extractZip, "Extract a zip file");
    component.def("compress_folder", &compressFolder, "Compress a folder");

    component.def("translate", &glob::translate, "Translate a pattern");
    component.def("compile_pattern", &glob::compilePattern,
                  "Compile a pattern");
    component.def("fnmatch", &glob::fnmatch,
                  "Check if a name matches a pattern");
    component.def("filter", &glob::filter, "Filter a list of names");
    component.def("expand_tilde", &glob::expandTilde, "Expand a tilde");
    component.def("has_magic", &glob::hasMagic, "Check if a pattern has magic");
    component.def("is_hidden", &glob::isHidden, "Check if a path is hidden");
    component.def("is_recursive", &glob::isRecursive,
                  "Check if a pattern is recursive");
    component.def("iter_dir", &glob::iterDirectory, "Iterate a directory");
    component.def("rlistdir", &glob::rlistdir, "Recursively list a directory");
    component.def<const std::string &>("glob", &glob::glob,
                                       "Glob a list of files");
    component.def<const std::string &>("rglob", &glob::rglob,
                                       "Recursively glob a list of files");
    component.def("glob0", &glob::glob0, "Glob0 a list of files");
    component.def("glob1", &glob::glob1, "Glob1 a list of files");
    component.def("glob2", &glob::glob2, "Glob2 a list of files");

    component.def(
        "mkdir",
        [](const std::string &path) -> bool { return createDirectory(path); },
        "Create a directory");
    component.def("mkdir_r", &createDirectoriesRecursive,
                  "Create a directory recursively");
    component.def("rmdir", &removeDirectory, "Remove a directory");
    component.def("rmdir_r", &removeDirectoriesRecursive,
                  "Remove a directory recursively");
    component.def("move", &moveDirectory, "Move a directory");
    component.def("rename", &renameDirectory, "Rename a directory");
    component.def("copy", &copyFile, "Copy a file");
    component.def("move_file", &moveFile, "Move a file");
    component.def("rename_file", &renameFile, "Rename a file");
    component.def("remove", &removeFile, "Remove a file");
    component.def("mksymlink", &createSymlink, "Create a symbolic link");
    component.def("rmsymlink", &removeSymlink, "Remove a symbolic link");

    DLOG_F(INFO, "Loaded module {}", component.getName());
});