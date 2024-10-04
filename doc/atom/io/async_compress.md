# Asynchronous Compression and Decompression Library Documentation

## Table of Contents

1. [Introduction](#introduction)
2. [Compression Classes](#compression-classes)
   - [BaseCompressor](#basecompressor)
   - [SingleFileCompressor](#singlefilecompressor)
   - [DirectoryCompressor](#directorycompressor)
3. [Decompression Classes](#decompression-classes)
   - [BaseDecompressor](#basedecompressor)
   - [SingleFileDecompressor](#singlefiledecompressor)
   - [DirectoryDecompressor](#directorydecompressor)
4. [ZIP Operations](#zip-operations)
   - [ListFilesInZip](#listfilesinzip)
   - [FileExistsInZip](#fileexistsinzip)
   - [RemoveFileFromZip](#removefilefromzip)
   - [GetZipFileSize](#getzipfilesize)
5. [Usage Examples](#usage-examples)
6. [Best Practices](#best-practices)

## Introduction

This library provides asynchronous compression and decompression functionality using the ASIO library for I/O operations and zlib for compression. It supports compressing and decompressing single files and directories, as well as various ZIP file operations.

## Compression Classes

### BaseCompressor

An abstract base class for compression operations.

#### Key Methods:

- `BaseCompressor(asio::io_context& io_context, const fs::path& output_file)`
- `virtual void start() = 0`

### SingleFileCompressor

Compresses a single file asynchronously.

#### Key Methods:

- `SingleFileCompressor(asio::io_context& io_context, const fs::path& input_file, const fs::path& output_file)`
- `void start() override`

### DirectoryCompressor

Compresses an entire directory asynchronously.

#### Key Methods:

- `DirectoryCompressor(asio::io_context& io_context, fs::path input_dir, const fs::path& output_file)`
- `void start() override`

## Decompression Classes

### BaseDecompressor

An abstract base class for decompression operations.

#### Key Methods:

- `explicit BaseDecompressor(asio::io_context& io_context)`
- `virtual void start() = 0`

### SingleFileDecompressor

Decompresses a single file asynchronously.

#### Key Methods:

- `SingleFileDecompressor(asio::io_context& io_context, fs::path input_file, fs::path output_folder)`
- `void start() override`

### DirectoryDecompressor

Decompresses multiple files from a directory asynchronously.

#### Key Methods:

- `DirectoryDecompressor(asio::io_context& io_context, const fs::path& input_dir, const fs::path& output_folder)`
- `void start() override`

## ZIP Operations

### ListFilesInZip

Lists files in a ZIP archive.

#### Key Methods:

- `ListFilesInZip(asio::io_context& io_context, std::string_view zip_file)`
- `void start() override`
- `[[nodiscard]] auto getFileList() const -> std::vector<std::string>`

### FileExistsInZip

Checks if a file exists in a ZIP archive.

#### Key Methods:

- `FileExistsInZip(asio::io_context& io_context, std::string_view zip_file, std::string_view file_name)`
- `void start() override`
- `[[nodiscard]] auto found() const -> bool`

### RemoveFileFromZip

Removes a file from a ZIP archive.

#### Key Methods:

- `RemoveFileFromZip(asio::io_context& io_context, std::string_view zip_file, std::string_view file_name)`
- `void start() override`
- `[[nodiscard]] auto isSuccessful() const -> bool`

### GetZipFileSize

Gets the size of a ZIP file.

#### Key Methods:

- `GetZipFileSize(asio::io_context& io_context, std::string_view zip_file)`
- `void start() override`
- `[[nodiscard]] auto getSizeValue() const -> size_t`

## Usage Examples

### Compressing a Single File

```cpp
#include "async_compress.hpp"
#include <asio.hpp>
#include <iostream>

int main() {
    asio::io_context io_context;

    atom::async::io::SingleFileCompressor compressor(
        io_context,
        "input.txt",
        "output.gz"
    );

    compressor.start();

    io_context.run();

    std::cout << "Compression completed." << std::endl;

    return 0;
}
```

### Compressing a Directory

```cpp
#include "async_compress.hpp"
#include <asio.hpp>
#include <iostream>

int main() {
    asio::io_context io_context;

    atom::async::io::DirectoryCompressor compressor(
        io_context,
        "input_directory",
        "output_archive.gz"
    );

    compressor.start();

    io_context.run();

    std::cout << "Directory compression completed." << std::endl;

    return 0;
}
```

### Decompressing a Single File

```cpp
#include "async_compress.hpp"
#include <asio.hpp>
#include <iostream>

int main() {
    asio::io_context io_context;

    atom::async::io::SingleFileDecompressor decompressor(
        io_context,
        "input.gz",
        "output_directory"
    );

    decompressor.start();

    io_context.run();

    std::cout << "Decompression completed." << std::endl;

    return 0;
}
```

### Listing Files in a ZIP Archive

```cpp
#include "async_compress.hpp"
#include <asio.hpp>
#include <iostream>

int main() {
    asio::io_context io_context;

    atom::async::io::ListFilesInZip list_files(
        io_context,
        "archive.zip"
    );

    list_files.start();

    io_context.run();

    for (const auto& file : list_files.getFileList()) {
        std::cout << file << std::endl;
    }

    return 0;
}
```

### Checking if a File Exists in a ZIP Archive

```cpp
#include "async_compress.hpp"
#include <asio.hpp>
#include <iostream>

int main() {
    asio::io_context io_context;

    atom::async::io::FileExistsInZip file_exists(
        io_context,
        "archive.zip",
        "file_to_check.txt"
    );

    file_exists.start();

    io_context.run();

    if (file_exists.found()) {
        std::cout << "File exists in the archive." << std::endl;
    } else {
        std::cout << "File does not exist in the archive." << std::endl;
    }

    return 0;
}
```
