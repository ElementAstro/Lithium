#ifndef ATOM_IO_SCRIPT_HPP
#define ATOM_IO_SCRIPT_HPP

#include "carbon/carbon.hpp"

#include "compress.hpp"
#include "file.hpp"
#include "glob.hpp"
#include "ifile.hpp"
#include "io.hpp"

using namespace Atom::IO;

namespace Atom::_Script::IO {
/**
 * Adds the String Methods to the given Carbon module.
 */
Carbon::ModulePtr bootstrap(
    Carbon::ModulePtr m = std::make_shared<Carbon::Module>()) {
    m->add(Carbon::fun(&compress_file), "compress_file");
    m->add(Carbon::fun(&decompress_file), "decompress_file");
    m->add(Carbon::fun(&compress_folder), "compress_folder");
    m->add(Carbon::fun(&create_zip), "create_zip");
    m->add(Carbon::fun(&extract_zip), "extract_zip");

    m->add(user_type<FileManager>(), "FileManager");
    m->add(Carbon::constructor<FileManager(const fs::path&)>(), "FileManager");
    m->add(Carbon::fun(&FileManager::createFile), "createFile");
    m->add(Carbon::fun(&FileManager::openFile), "openFile");
    m->add(Carbon::fun(&FileManager::readFile), "readFile");
    m->add(Carbon::fun(&FileManager::writeFile), "writeFile");
    m->add(Carbon::fun(&FileManager::moveFile), "moveFile");
    m->add(Carbon::fun(&FileManager::deleteFile), "deleteFile");
    m->add(Carbon::fun(&FileManager::getFileSize), "getFileSize");
    m->add(Carbon::fun(&FileManager::getFileDirectory), "getFileDirectory");

    m->add(Carbon::fun(&glob::translate), "translate");
    m->add(Carbon::fun(&glob::expand_tilde), "expand_tilde");
    m->add(Carbon::fun(&glob::has_magic), "has_magic");
    m->add(Carbon::fun(&glob::is_hidden), "is_hidden");
    m->add(Carbon::fun(&glob::string_replace), "string_replace");
    m->add(Carbon::fun(&glob::is_recursive), "is_recursive");
    m->add(Carbon::fun(&glob::filter), "filter");
    m->add(Carbon::fun(&glob::glob0), "glob0");
    m->add(Carbon::fun(&glob::compile_pattern), "compile_pattern");
    m->add(Carbon::fun(&glob::glob1), "glob1");
    m->add(Carbon::fun(&glob::glob2), "glob2");
    m->add(Carbon::fun(&glob::iter_directory), "iter_directory");
    m->add(Carbon::fun(&glob::rlistdir), "rlistdir");

    m->add(user_type<FileWrapper>(), "FileWrapper");
    m->add(Carbon::constructor<FileWrapper(const fs::path&)>(), "FileWrapper");
    m->add(Carbon::fun(
               static_cast<void (FileWrapper::*)(const std::vector<uint8_t>&)>(
                   &FileWrapper::write)),
           "write");
    m->add(Carbon::fun(&FileWrapper::read), "read");
    m->add(Carbon::fun(&FileWrapper::exists), "exists");
    m->add(Carbon::fun(&FileWrapper::remove), "remove");
    m->add(Carbon::fun(&FileWrapper::get_path), "get_path");
    m->add(Carbon::fun(&FileWrapper::is_binary_file), "is_binary_file");
    m->add(Carbon::fun(&FileWrapper::get_size), "get_size");
    m->add(Carbon::fun(&FileWrapper::get_size_string), "get_size_string");
    m->add(Carbon::fun(&FileWrapper::get_last_write_time),
           "get_last_write_time");
    m->add(Carbon::fun(&FileWrapper::rename), "rename");
    m->add(Carbon::fun(&FileWrapper::copy_to), "copy_to");
    m->add(Carbon::fun(&FileWrapper::move_to), "move_to");
    m->add(Carbon::fun(&FileWrapper::is_empty), "is_empty");
    m->add(Carbon::fun(
               static_cast<void (FileWrapper::*)(const std::vector<uint8_t>&)>(
                   &FileWrapper::append)),
           "append");

    m->add(Carbon::fun(
               static_cast<bool (*)(const std::string&)>(&createDirectory)),
           "createDirectory");
    m->add(Carbon::fun(&createDirectoriesRecursive),
           "createDirectoriesRecursive");
    m->add(Carbon::fun(&removeDirectory), "removeDirectory");
    m->add(Carbon::fun(&renameDirectory), "renameDirectory");
    m->add(Carbon::fun(&moveDirectory), "moveDirectory");
    m->add(Carbon::fun(&copyFile), "copyFile");
    m->add(Carbon::fun(&moveFile), "moveFile");
    m->add(Carbon::fun(&removeFile), "removeFile");
    m->add(Carbon::fun(&createSymlink), "createSymlink");
    m->add(Carbon::fun(&removeSymlink), "removeSymlink");
    m->add(Carbon::fun(&fileSize), "fileSize");
    m->add(Carbon::fun(&traverseDirectories), "traverseDirectories");
    m->add(Carbon::fun(&checkFileTypeInFolder), "checkFileTypeInFolder");
    m->add(
        Carbon::fun(static_cast<bool (*)(const std::string&)>(&isFolderExists)),
        "isFolderExists");
    m->add(Carbon::fun(static_cast<bool (*)(const fs::path&)>(&isFolderExists)),
           "isFolderExists");
    m->add(Carbon::fun(&isFileExists), "isFileExists");
    m->add(Carbon::fun(&isFolderNameValid), "isFolderNameValid");
    m->add(Carbon::fun(&isFileNameValid), "isFileNameValid");
    m->add(Carbon::fun(&isAbsolutePath), "isAbsolutePath");
    m->add(Carbon::fun(&isExecutableFile), "isExecutableFile");
    m->add(Carbon::fun(&changeWorkingDirectory), "changeWorkingDirectory");
    m->add(Carbon::fun(&convertToLinuxPath), "convertToLinuxPath");
    m->add(Carbon::fun(&convertToWindowsPath), "convertToWindowsPath");
    m->add(Carbon::fun(&normPath), "normPath");
    m->add(Carbon::fun(&isFolderEmpty), "isFolderEmpty");
    m->add(Carbon::fun(&getFileTimes), "getFileTimes");
    m->add(Carbon::fun(&renameFile), "renameFile");
    return m;
}
}  // namespace Atom::_Script::IO

#endif