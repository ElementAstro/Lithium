#include <pybind11/pybind11.h>

#include "compress.hpp"
#include "file.hpp"
#include "glob.hpp"
#include "idirectory.hpp"
#include "ifile.hpp"
#include "io.hpp"

namespace py = pybind11;

using namespace atom::io;

PYBIND11_MODULE(atom_io, m) {
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

    py::class_<DirectoryWrapper>(m, "DirectoryWrapper")
        .def("get_path", &DirectoryWrapper::get_path,
             "Get the path of a "
             "directory")
        .def("get_size", &DirectoryWrapper::get_size,
             "Get the size of a "
             "directory")
        .def("get_size_string", &DirectoryWrapper::get_size_string,
             "Get the size of a directory as a string")
        .def("list_files", &DirectoryWrapper::list_files,
             "List the files of a "
             "directory")
        .def("list_directories", &DirectoryWrapper::list_directories,
             "List the directories of a directory")
        .def("create_directory", &DirectoryWrapper::create_directory,
             "Create a directory");

    py::class_<FileWrapper>(m, "FileWrapper")
        .def("get_path", &FileWrapper::get_path, "Get the path of a file")
        .def("get_size", &FileWrapper::get_size, "Get the size of a file")
        .def("get_size_string", &FileWrapper::get_size_string,
             "Get the size of a file as a string")
        .def("get_parent_path", &FileWrapper::get_parent_path,
             "Get the parent path of a file")
        .def("get_extension", &FileWrapper::get_extension,
             "Get the extension of a file")
        .def("get_stem", &FileWrapper::get_stem, "Get the stem of a file")
        .def("get_last_write_time", &FileWrapper::get_last_write_time,
             "Get the last write time of a file")
        .def("get_hard_link_count", &FileWrapper::get_hard_link_count,
             "Get the hard link count of a file")
        .def("is_directory", &FileWrapper::is_directory,
             "Check if a file is a directory")
        .def("is_regular_file", &FileWrapper::is_regular_file,
             "Check if a file is a regular file")
        .def("is_binary_file", &FileWrapper::is_binary_file,
             "Check if a file is a binary file")
        .def("is_symlink", &FileWrapper::is_symlink,
             "Check if a file is a symlink")
        .def("exists", &FileWrapper::exists, "Check if a file exists")
        .def("rename", &FileWrapper::rename, "Rename a file");

    m.def("is_folder_exists",
          py::overload_cast<const std::string &>(&isFolderExists),
          "Check if a folder exists");
    m.def("is_folder_name_valid", &isFolderNameValid,
          "Check if a folder name is valid");
    m.def("is_file_exists",
          py::overload_cast<const std::string &>(&isFileExists),
          "Check if a file exists");
    m.def("is_file_name_valid", &isFileNameValid,
          "Check if a file name is valid");
    m.def("is_absolute_path", &isAbsolutePath, "Check if a path is absolute");
    m.def("is_executable_file", &isExecutableFile,
          "Check if a file is executable");
    m.def("is_folder_empty", &isFolderEmpty, "Check if a folder is empty");
}
