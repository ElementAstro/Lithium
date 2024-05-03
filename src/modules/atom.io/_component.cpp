/*
 * _component.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-13

Description: Component of Atom-System

**************************************************/

#include "_component.hpp"

#include "atom/log/loguru.hpp"

#include "atom/io/compress.hpp"
#include "atom/io/file.hpp"
#include "atom/io/glob.hpp"
#include "atom/io/io.hpp"

using namespace atom::io;

IOComponent::IOComponent(const std::string &name) : Component(name) {
    DLOG_F(INFO, "IOComponent::IOComponent");

    def("compress", &compress_file, "Compress a file");
    def("decompress", &decompress_file, "Decompress a file");
    def("create_zip", &create_zip, "Create a zip file");
    def("extract_zip", &extract_zip, "Extract a zip file");
    def("compress_folder", &compress_folder, "Compress a folder");

    def("translate", &glob::translate, "Translate a pattern");
    def("compile_pattern", &glob::compile_pattern, "Compile a pattern");
    def("fnmatch", &glob::fnmatch, "Check if a name matches a pattern");
    def("filter", &glob::filter, "Filter a list of names");
    def("expand_tilde", &glob::expand_tilde, "Expand a tilde");
    def("has_magic", &glob::has_magic, "Check if a pattern has magic");
    def("is_hidden", &glob::is_hidden, "Check if a path is hidden");
    def("is_recursive", &glob::is_recursive, "Check if a pattern is recursive");
    def("iter_dir", &glob::iter_directory, "Iterate a directory");
    def("rlistdir", &glob::rlistdir, "Recursively list a directory");
    def<const std::string &>("glob", &glob::glob, "Glob a list of files");
    def<const std::string &>("rglob", &glob::rglob,
                             "Recursively glob a list of files");
    def("glob0", &glob::glob0, "Glob0 a list of files");
    def("glob1", &glob::glob1, "Glob1 a list of files");
    def("glob2", &glob::glob2, "Glob2 a list of files");

    def(
        "mkdir",
        [](const std::string &path) -> bool { return createDirectory(path); },
        "Create a directory");
    def("mkdir_r", &createDirectoriesRecursive,
        "Create a directory recursively");
    def("rmdir", &removeDirectory, "Remove a directory");
    def("rmdir_r", &removeDirectoriesRecursive,
        "Remove a directory recursively");
    def("move", &moveDirectory, "Move a directory");
    def("rename", &renameDirectory, "Rename a directory");
    def("copy", &copyFile, "Copy a file");
    def("move_file", &moveFile, "Move a file");
    def("rename_file", &renameFile, "Rename a file");
    def("remove", &removeFile, "Remove a file");
    def("mksymlink", &createSymlink, "Create a symbolic link");
    def("rmsymlink", &removeSymlink, "Remove a symbolic link");
}

IOComponent::~IOComponent() { DLOG_F(INFO, "IOComponent::~IOComponent"); }

bool IOComponent::initialize() { return true; }

bool IOComponent::destroy() { return true; }
