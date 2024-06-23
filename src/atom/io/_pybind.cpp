#include <pybind11/pybind11.h>

#include "compress.hpp"
#include "file.hpp"
#include "glob.hpp"
#include "io.hpp"

namespace py = pybind11;

using namespace atom::io;

PYBIND11_EMBEDDED_MODULE(atom_io, m) {
    m.doc() = "atom_io module";

    m.def("compress_file", &compress_file, "Compress a file");
    m.def("decompress_file", &decompress_file, "Decompress a file");
    m.def("compress_folder", &compress_folder, "Compress a folder");
    m.def("extract_zip", &extract_zip, "Extract a zip file");
    m.def("create_zip", &create_zip, "Create a zip file");

    py::class_<FileManager>(m, "FileManager")
        .def("create_file", &FileManager::createFile, "Create a file")
        .def("open_file", &FileManager::openFile, "Open a file")
        .def("read_file", &FileManager::readFile, "Read a file")
        .def("write_file", &FileManager::writeFile, "Write a file")
        .def("move_file", &FileManager::moveFile, "Move a file")
        .def("delete_file", &FileManager::deleteFile, "Delete a file")
        .def("get_file_size", &FileManager::getFileSize,
             "Get the size of a file")
        .def("get_file_directory", &FileManager::getFileDirectory,
             "Get the directory of a file");

    m.def("translate", &glob::translate, "Translate a pattern");
    m.def("expand_tilde", &glob::expand_tilde, "Expand a tilde");
    m.def("has_magic", &glob::has_magic, "Check if a pattern has magic");
    m.def("is_hidden", &glob::is_hidden, "Check if a file is hidden");
    m.def("string_replace", &glob::string_replace, "Replace a string");
    m.def("is_recursive", &glob::is_recursive,
          "Check if a pattern is recursive");
    m.def("filter", &glob::filter, "Filter a list of files");
    m.def("glob0", &glob::glob0, "Glob0");
    m.def("compile_pattern", &glob::compile_pattern, "Compile a pattern");
    m.def("glob1", &glob::glob1, "Glob1");
    m.def("glob2", &glob::glob2, "Glob2");
    m.def("iter_directory", &glob::iter_directory, "Iterate over a directory");
    m.def("rlistdir", &glob::rlistdir, "List a directory recursively");

    py::class_<CreateDirectoriesOptions>(m, "CreateDirectoriesOptions")
        .def(py::init<>())
        .def_readwrite("verbose", &CreateDirectoriesOptions::verbose)
        .def_readwrite("dry_run", &CreateDirectoriesOptions::dryRun)
        .def_readwrite("delay", &CreateDirectoriesOptions::delay)
        .def_readwrite("filter", &CreateDirectoriesOptions::filter)
        .def_readwrite("on_create", &CreateDirectoriesOptions::onCreate)
        .def_readwrite("on_delete", &CreateDirectoriesOptions::onDelete);

    m.def("create_dirs_r", &createDirectoriesRecursive,
          "Create directories recursively");
    m.def("remove_disr_r", &removeDirectoriesRecursive,
          "Remove directories recursively");
    m.def("is_folder_exists",
          py::overload_cast<const std::string &>(&isFolderExists),
          "Check if a folder exists");
    m.def("is_folder_name_valid", &isFolderNameValid,
          "Check if a folder name is valid");
    m.def("is_file_exists",
          py::overload_cast<const std::string &>(&isFileExists),
          "Check if a file exists");

    py::enum_<FileOption>(m, "FileOption")
        .value("Path", FileOption::Path)
        .value("Name", FileOption::Name);

    m.def("check_type", &checkFileTypeInFolder, "Check file type in folder");
    m.def("is_file_name_valid", &isFileNameValid,
          "Check if a file name is valid");
    m.def("convert_to_windows_path", &convertToWindowsPath,
          "Convert to windows path");
    m.def("convert_to_linux_path", &convertToLinuxPath,
          "Convert to linux path");
    m.def("jwalk", &jwalk, "Walk a folder");
    m.def("fwalk", &fwalk, "Walk a folder");
    m.def("norm_path", &normPath, "Normalize a path");
    m.def("file_size", &fileSize, "Get file size");
    m.def("remove_file", &removeFile, "Remove a file");
    m.def("rename_file", &renameFile, "Rename a file");
    m.def("truncate_file", &truncateFile, "Truncate a file");
    m.def("move_file", &moveFile, "Move a file");
    m.def("remove_directory", &removeDirectory, "Remove a directory");
    m.def("create_symlink", &createSymlink, "Create a symbolic link");
    m.def("remove_symlink", &removeSymlink, "Remove a symbolic link");
    m.def("rename_directory", &renameDirectory, "Rename a directory");
    m.def("get_file_times", &getFileTimes, "Get file times");
    m.def("move_directory", &moveDirectory, "Move a directory");
    m.def("copy_file", &copyFile, "Copy a file");

    m.def("is_absolute_path", &isAbsolutePath, "Check if a path is absolute");
    m.def("cwdir", &changeWorkingDirectory, "Change working directory");
    m.def("is_executable_file", &isExecutableFile,
          "Check if a file is executable");
    m.def("is_folder_empty", &isFolderEmpty, "Check if a folder is empty");
}
