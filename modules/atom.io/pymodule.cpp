#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "atom/io/async_compress.hpp"
#include "atom/io/async_glob.hpp"
#include "atom/io/async_io.hpp"
#include "atom/io/compress.hpp"
#include "atom/io/glob.hpp"
#include "atom/io/io.hpp"
#include "atom/io/pushd.hpp"

namespace py = pybind11;

PYBIND11_MODULE(io, m) {
    m.doc() = "Python bindings for Atom IO Module";

    py::enum_<atom::io::PathType>(m, "path_type")
        .value("NOT_EXISTS", atom::io::PathType::NOT_EXISTS)
        .value("REGULAR_FILE", atom::io::PathType::REGULAR_FILE)
        .value("DIRECTORY", atom::io::PathType::DIRECTORY)
        .value("SYMLINK", atom::io::PathType::SYMLINK)
        .value("OTHER", atom::io::PathType::OTHER);

    py::class_<atom::io::CreateDirectoriesOptions>(m,
                                                   "create_directories_options")
        .def(py::init<>())
        .def_readwrite("verbose", &atom::io::CreateDirectoriesOptions::verbose)
        .def_readwrite("dry_run", &atom::io::CreateDirectoriesOptions::dryRun)
        .def_readwrite("delay", &atom::io::CreateDirectoriesOptions::delay)
        .def_readwrite("filter", &atom::io::CreateDirectoriesOptions::filter)
        .def_readwrite("on_create",
                       &atom::io::CreateDirectoriesOptions::onCreate)
        .def_readwrite("on_delete",
                       &atom::io::CreateDirectoriesOptions::onDelete);

    m.def("create_directory",
          py::overload_cast<const std::string &>(&atom::io::createDirectory),
          "Create a directory", py::arg("path"), py::arg("root_dir") = "");

    m.def("create_directories_recursive", &atom::io::createDirectoriesRecursive,
          "Create directories recursively", py::arg("base_path"),
          py::arg("subdirs"), py::arg("options"));

    m.def("remove_directory", &atom::io::removeDirectory, "Remove a directory",
          py::arg("path"));

    m.def("remove_directories_recursive", &atom::io::removeDirectoriesRecursive,
          "Remove directories recursively", py::arg("base_path"),
          py::arg("subdirs"),
          py::arg("options") = atom::io::CreateDirectoriesOptions());

    m.def("rename_directory", &atom::io::renameDirectory, "Rename a directory",
          py::arg("old_path"), py::arg("new_path"));

    m.def("move_directory", &atom::io::moveDirectory, "Move a directory",
          py::arg("old_path"), py::arg("new_path"));

    m.def("copy_file", &atom::io::copyFile, "Copy a file", py::arg("src_path"),
          py::arg("dst_path"));

    m.def("move_file", &atom::io::moveFile, "Move a file", py::arg("src_path"),
          py::arg("dst_path"));

    m.def("rename_file", &atom::io::renameFile, "Rename a file",
          py::arg("old_path"), py::arg("new_path"));

    m.def("remove_file", &atom::io::removeFile, "Remove a file",
          py::arg("path"));

    m.def("create_symlink", &atom::io::createSymlink, "Create a symbolic link",
          py::arg("target_path"), py::arg("symlink_path"));

    m.def("remove_symlink", &atom::io::removeSymlink, "Remove a symbolic link",
          py::arg("path"));

    m.def("file_size", &atom::io::fileSize, "Get the size of a file",
          py::arg("path"));

    m.def("truncate_file", &atom::io::truncateFile, "Truncate a file",
          py::arg("path"), py::arg("size"));

    m.def("jwalk", &atom::io::jwalk, "Recursively walk through a directory",
          py::arg("root"));

    m.def("fwalk", &atom::io::fwalk, "Recursively walk through a directory",
          py::arg("root"), py::arg("callback"));

    m.def("convert_to_linux_path", &atom::io::convertToLinuxPath,
          "Convert Windows path to Linux path", py::arg("windows_path"));

    m.def("convert_to_windows_path", &atom::io::convertToWindowsPath,
          "Convert Linux path to Windows path", py::arg("linux_path"));

    m.def("norm_path", &atom::io::normPath, "Normalize a path",
          py::arg("raw_path"));

    m.def("is_folder_name_valid", &atom::io::isFolderNameValid,
          "Check if the folder name is valid", py::arg("folder_name"));

    m.def("is_file_name_valid", &atom::io::isFileNameValid,
          "Check if the file name is valid", py::arg("file_name"));

    m.def("is_folder_exists", &atom::io::isFolderExists,
          "Check if the folder exists", py::arg("folder_name"));

    m.def("is_file_exists", &atom::io::isFileExists, "Check if the file exists",
          py::arg("file_name"));

    m.def("is_folder_empty", &atom::io::isFolderEmpty,
          "Check if the folder is empty", py::arg("folder_name"));

    m.def("is_absolute_path", &atom::io::isAbsolutePath,
          "Check if the path is an absolute path", py::arg("path"));

    m.def("change_working_directory", &atom::io::changeWorkingDirectory,
          "Change the working directory", py::arg("directory_path"));

    m.def("get_file_times", &atom::io::getFileTimes, "Get the file times",
          py::arg("file_path"));

    py::enum_<atom::io::FileOption>(m, "file_option")
        .value("PATH", atom::io::FileOption::PATH)
        .value("NAME", atom::io::FileOption::NAME);

    m.def("check_file_type_in_folder", &atom::io::checkFileTypeInFolder,
          "Check the file type in the folder", py::arg("folder_path"),
          py::arg("file_types"), py::arg("file_option"));

    m.def("is_executable_file", &atom::io::isExecutableFile,
          "Check whether the specified file exists", py::arg("file_name"),
          py::arg("file_ext"));

    m.def("get_file_size", &atom::io::getFileSize, "Get the file size",
          py::arg("file_path"));

    m.def("calculate_chunk_size", &atom::io::calculateChunkSize,
          "Calculate the chunk size", py::arg("file_size"),
          py::arg("num_chunks"));

    m.def("split_file", &atom::io::splitFile,
          "Split a file into multiple parts", py::arg("file_path"),
          py::arg("chunk_size"), py::arg("output_pattern") = "");

    m.def("merge_files", &atom::io::mergeFiles,
          "Merge multiple parts into a single file",
          py::arg("output_file_path"), py::arg("part_files"));

    m.def("quick_split", &atom::io::quickSplit,
          "Quickly split a file into multiple parts", py::arg("file_path"),
          py::arg("num_chunks"), py::arg("output_pattern") = "");

    m.def("quick_merge", &atom::io::quickMerge,
          "Quickly merge multiple parts into a single file",
          py::arg("output_file_path"), py::arg("part_pattern"),
          py::arg("num_chunks"));

    m.def("get_executable_name_from_path", &atom::io::getExecutableNameFromPath,
          "Get the executable name from the path", py::arg("path"));

    m.def("check_path_type", &atom::io::checkPathType, "Get the file type",
          py::arg("path"));

    m.def("count_lines_in_file", &atom::io::countLinesInFile,
          "Count lines in a file", py::arg("file_path"));

    m.def("search_executable_files", &atom::io::searchExecutableFiles,
          "Search executable files", py::arg("dir"), py::arg("search_str"));

    m.def("compress_file", &atom::io::compressFile, "Compress a single file",
          py::arg("file_name"), py::arg("output_folder"));

    m.def("decompress_file", &atom::io::decompressFile,
          "Decompress a single file", py::arg("file_name"),
          py::arg("output_folder"));

    m.def("compress_folder", &atom::io::compressFolder,
          "Compress all files in a specified directory",
          py::arg("folder_name"));

    m.def("extract_zip", &atom::io::extractZip, "Extract a single ZIP file",
          py::arg("zip_file"), py::arg("destination_folder"));

    m.def("create_zip", &atom::io::createZip, "Create a ZIP file",
          py::arg("source_folder"), py::arg("zip_file"),
          py::arg("compression_level") = -1);

    m.def("list_files_in_zip", &atom::io::listFilesInZip,
          "List files in a ZIP file", py::arg("zip_file"));

    m.def("file_exists_in_zip", &atom::io::fileExistsInZip,
          "Check if a specified file exists in a ZIP file", py::arg("zip_file"),
          py::arg("file_name"));

    m.def("remove_file_from_zip", &atom::io::removeFileFromZip,
          "Remove a specified file from a ZIP file", py::arg("zip_file"),
          py::arg("file_name"));

    m.def("get_zip_file_size", &atom::io::getZipFileSize,
          "Get the size of a file in a ZIP file", py::arg("zip_file"));

    py::class_<atom::io::DirectoryStack>(m, "DirectoryStack")
        .def(py::init<asio::io_context &>(), py::arg("io_context"))
        .def("async_pushd", &atom::io::DirectoryStack::asyncPushd,
             "Push the current directory onto the stack and change to the "
             "specified directory asynchronously",
             py::arg("new_dir"), py::arg("handler"))
        .def("async_popd", &atom::io::DirectoryStack::asyncPopd,
             "Pop the directory from the stack and change back to it "
             "asynchronously",
             py::arg("handler"))
        .def("peek", &atom::io::DirectoryStack::peek,
             "View the top directory in the stack without changing to it")
        .def("dirs", &atom::io::DirectoryStack::dirs,
             "Display the current stack of directories")
        .def("clear", &atom::io::DirectoryStack::clear,
             "Clear the directory stack")
        .def("swap", &atom::io::DirectoryStack::swap,
             "Swap two directories in the stack given their indices",
             py::arg("index1"), py::arg("index2"))
        .def("remove", &atom::io::DirectoryStack::remove,
             "Remove a directory from the stack at the specified index",
             py::arg("index"))
        .def("async_goto_index", &atom::io::DirectoryStack::asyncGotoIndex,
             "Change to the directory at the specified index in the stack "
             "asynchronously",
             py::arg("index"), py::arg("handler"))
        .def("async_save_stack_to_file",
             &atom::io::DirectoryStack::asyncSaveStackToFile,
             "Save the directory stack to a file asynchronously",
             py::arg("filename"), py::arg("handler"))
        .def("async_load_stack_from_file",
             &atom::io::DirectoryStack::asyncLoadStackFromFile,
             "Load the directory stack from a file asynchronously",
             py::arg("filename"), py::arg("handler"))
        .def("size", &atom::io::DirectoryStack::size,
             "Get the size of the directory stack")
        .def("is_empty", &atom::io::DirectoryStack::isEmpty,
             "Check if the directory stack is empty")
        .def("async_get_current_directory",
             &atom::io::DirectoryStack::asyncGetCurrentDirectory,
             "Get the current directory path asynchronously",
             py::arg("handler"));

    m.def("string_replace", &atom::io::stringReplace,
          "Replace a substring in a string", py::arg("str"), py::arg("from"),
          py::arg("to_str"));

    m.def("translate", &atom::io::translate,
          "Translate a pattern to a regex string", py::arg("pattern"));

    m.def("compile_pattern", &atom::io::compilePattern,
          "Compile a pattern to a regex", py::arg("pattern"));

    m.def("fnmatch", &atom::io::fnmatch, "Match a filename against a pattern",
          py::arg("name"), py::arg("pattern"));

    m.def("filter", &atom::io::filter,
          "Filter a list of names against a pattern", py::arg("names"),
          py::arg("pattern"));

    m.def("expand_tilde", &atom::io::expandTilde, "Expand tilde in a path",
          py::arg("path"));

    m.def("has_magic", &atom::io::hasMagic,
          "Check if a pathname contains any magic characters",
          py::arg("pathname"));

    m.def("is_hidden", &atom::io::isHidden, "Check if a pathname is hidden",
          py::arg("pathname"));

    m.def("is_recursive", &atom::io::isRecursive,
          "Check if a pattern is recursive", py::arg("pattern"));

    m.def("iter_directory", &atom::io::iterDirectory,
          "Iterate over a directory", py::arg("dirname"), py::arg("dironly"));

    m.def("rlistdir", &atom::io::rlistdir, "Recursively list a directory",
          py::arg("dirname"), py::arg("dironly"));

    m.def("glob2", &atom::io::glob2, "Recursive glob", py::arg("dirname"),
          py::arg("pattern"), py::arg("dironly"));

    m.def("glob1", &atom::io::glob1, "Non-recursive glob", py::arg("dirname"),
          py::arg("pattern"), py::arg("dironly"));

    m.def("glob0", &atom::io::glob0, "Glob with no magic", py::arg("dirname"),
          py::arg("basename"), py::arg("dironly"));

    m.def("glob",
          py::overload_cast<const std::string &, bool, bool>(&atom::io::glob),
          "Glob with pathname", py::arg("pathname"),
          py::arg("recursive") = false, py::arg("dironly") = false);

    m.def("glob",
          py::overload_cast<const std::vector<std::string> &>(&atom::io::glob),
          "Glob with pathnames", py::arg("pathnames"));

    m.def("rglob", py::overload_cast<const std::string &>(&atom::io::rglob),
          "Recursive glob with pathname", py::arg("pathname"));

    m.def("rglob",
          py::overload_cast<const std::vector<std::string> &>(&atom::io::rglob),
          "Recursive glob with pathnames", py::arg("pathnames"));

    m.def("glob",
          py::overload_cast<const std::initializer_list<std::string> &>(
              &atom::io::glob),
          "Glob with initializer list", py::arg("pathnames"));

    m.def("rglob",
          py::overload_cast<const std::initializer_list<std::string> &>(
              &atom::io::rglob),
          "Recursive glob with initializer list", py::arg("pathnames"));

    py::class_<atom::async::io::BaseCompressor>(m, "BaseCompressor")
        .def("start", &atom::async::io::BaseCompressor::start,
             "Start the compression process");

    py::class_<atom::async::io::SingleFileCompressor,
               atom::async::io::BaseCompressor>(m, "SingleFileCompressor")
        .def(py::init<asio::io_context &, const std::filesystem::path &,
                      const std::filesystem::path &>(),
             py::arg("io_context"), py::arg("input_file"),
             py::arg("output_file"))
        .def("start", &atom::async::io::SingleFileCompressor::start,
             "Start the compression process");

    py::class_<atom::async::io::DirectoryCompressor,
               atom::async::io::BaseCompressor>(m, "DirectoryCompressor")
        .def(py::init<asio::io_context &, const std::filesystem::path &,
                      const std::filesystem::path &>(),
             py::arg("io_context"), py::arg("input_dir"),
             py::arg("output_file"))
        .def("start", &atom::async::io::DirectoryCompressor::start,
             "Start the compression process");

    py::class_<atom::async::io::BaseDecompressor>(m, "BaseDecompressor")
        .def("start", &atom::async::io::BaseDecompressor::start,
             "Start the decompression process");

    py::class_<atom::async::io::SingleFileDecompressor,
               atom::async::io::BaseDecompressor>(m, "SingleFileDecompressor")
        .def(py::init<asio::io_context &, const std::filesystem::path &,
                      const std::filesystem::path &>(),
             py::arg("io_context"), py::arg("input_file"),
             py::arg("output_folder"))
        .def("start", &atom::async::io::SingleFileDecompressor::start,
             "Start the decompression process");

    py::class_<atom::async::io::DirectoryDecompressor,
               atom::async::io::BaseDecompressor>(m, "DirectoryDecompressor")
        .def(py::init<asio::io_context &, const std::filesystem::path &,
                      const std::filesystem::path &>(),
             py::arg("io_context"), py::arg("input_dir"),
             py::arg("output_folder"))
        .def("start", &atom::async::io::DirectoryDecompressor::start,
             "Start the decompression process");

    py::class_<atom::async::io::ZipOperation>(m, "ZipOperation")
        .def("start", &atom::async::io::ZipOperation::start,
             "Start the ZIP operation");

    py::class_<atom::async::io::ListFilesInZip, atom::async::io::ZipOperation>(
        m, "ListFilesInZip")
        .def(py::init<asio::io_context &, std::string_view>(),
             py::arg("io_context"), py::arg("zip_file"))
        .def("start", &atom::async::io::ListFilesInZip::start,
             "Start the ZIP operation")
        .def("get_file_list", &atom::async::io::ListFilesInZip::getFileList,
             "Get the list of files in the ZIP archive");

    py::class_<atom::async::io::FileExistsInZip, atom::async::io::ZipOperation>(
        m, "FileExistsInZip")
        .def(py::init<asio::io_context &, std::string_view, std::string_view>(),
             py::arg("io_context"), py::arg("zip_file"), py::arg("file_name"))
        .def("start", &atom::async::io::FileExistsInZip::start,
             "Start the ZIP operation")
        .def("found", &atom::async::io::FileExistsInZip::found,
             "Check if the file was found in the ZIP archive");

    py::class_<atom::async::io::RemoveFileFromZip,
               atom::async::io::ZipOperation>(m, "RemoveFileFromZip")
        .def(py::init<asio::io_context &, std::string_view, std::string_view>(),
             py::arg("io_context"), py::arg("zip_file"), py::arg("file_name"))
        .def("start", &atom::async::io::RemoveFileFromZip::start,
             "Start the ZIP operation")
        .def("is_successful", &atom::async::io::RemoveFileFromZip::isSuccessful,
             "Check if the file removal was successful");

    py::class_<atom::async::io::GetZipFileSize, atom::async::io::ZipOperation>(
        m, "GetZipFileSize")
        .def(py::init<asio::io_context &, std::string_view>(),
             py::arg("io_context"), py::arg("zip_file"))
        .def("start", &atom::async::io::GetZipFileSize::start,
             "Start the ZIP operation")
        .def("get_size_value", &atom::async::io::GetZipFileSize::getSizeValue,
             "Get the size of the ZIP file");

    py::class_<atom::io::AsyncGlob>(m, "AsyncGlob")
        .def(py::init<asio::io_context &>(), py::arg("io_context"))
        .def("glob", &atom::io::AsyncGlob::glob,
             "Perform a glob operation to match files", py::arg("pathname"),
             py::arg("callback"), py::arg("recursive") = false,
             py::arg("dironly") = false);

    py::class_<atom::async::io::AsyncFile>(m, "AsyncFile")
        .def(py::init<asio::io_context &>(), py::arg("io_context"))
        .def("async_read", &atom::async::io::AsyncFile::asyncRead,
             "Asynchronously read the content of a file", py::arg("filename"),
             py::arg("callback"))
        .def("async_write", &atom::async::io::AsyncFile::asyncWrite,
             "Asynchronously write content to a file", py::arg("filename"),
             py::arg("content"), py::arg("callback"))
        .def("async_delete", &atom::async::io::AsyncFile::asyncDelete,
             "Asynchronously delete a file", py::arg("filename"),
             py::arg("callback"))
        .def("async_copy", &atom::async::io::AsyncFile::asyncCopy,
             "Asynchronously copy a file", py::arg("src"), py::arg("dest"),
             py::arg("callback"))
        .def("async_read_with_timeout",
             &atom::async::io::AsyncFile::asyncReadWithTimeout,
             "Asynchronously read the content of a file with a timeout",
             py::arg("filename"), py::arg("timeoutMs"), py::arg("callback"))
        .def("async_batch_read", &atom::async::io::AsyncFile::asyncBatchRead,
             "Asynchronously read the content of multiple files",
             py::arg("files"), py::arg("callback"))
        .def("async_stat", &atom::async::io::AsyncFile::asyncStat,
             "Asynchronously retrieve the status of a file",
             py::arg("filename"), py::arg("callback"))
        .def("async_move", &atom::async::io::AsyncFile::asyncMove,
             "Asynchronously move a file", py::arg("src"), py::arg("dest"),
             py::arg("callback"))
        .def("async_change_permissions",
             &atom::async::io::AsyncFile::asyncChangePermissions,
             "Asynchronously change the permissions of a file",
             py::arg("filename"), py::arg("perms"), py::arg("callback"))
        .def("async_create_directory",
             &atom::async::io::AsyncFile::asyncCreateDirectory,
             "Asynchronously create a directory", py::arg("path"),
             py::arg("callback"))
        .def("async_exists", &atom::async::io::AsyncFile::asyncExists,
             "Asynchronously check if a file exists", py::arg("filename"),
             py::arg("callback"));

    py::class_<atom::async::io::AsyncDirectory>(m, "AsyncDirectory")
        .def(py::init<asio::io_context &>(), py::arg("io_context"))
        .def("async_create", &atom::async::io::AsyncDirectory::asyncCreate,
             "Asynchronously create a directory", py::arg("path"),
             py::arg("callback"))
        .def("async_remove", &atom::async::io::AsyncDirectory::asyncRemove,
             "Asynchronously remove a directory", py::arg("path"),
             py::arg("callback"))
        .def("async_list_contents",
             &atom::async::io::AsyncDirectory::asyncListContents,
             "Asynchronously list the contents of a directory", py::arg("path"),
             py::arg("callback"))
        .def("async_exists", &atom::async::io::AsyncDirectory::asyncExists,
             "Asynchronously check if a directory exists", py::arg("path"),
             py::arg("callback"));
}