# Compression and ZIP Operations Documentation

## Table of Contents

1. [Introduction](#introduction)
2. [File Compression and Decompression](#file-compression-and-decompression)
   - [compressFile](#compressfile)
   - [decompressFile](#decompressfile)
   - [compressFolder](#compressfolder)
3. [ZIP File Operations](#zip-file-operations)
   - [extractZip](#extractzip)
   - [createZip](#createzip)
   - [listFilesInZip](#listfilesinzip)
   - [fileExistsInZip](#fileexistsinzip)
   - [removeFileFromZip](#removefilefromzip)
   - [getZipFileSize](#getzipfilesize)
4. [Usage Examples](#usage-examples)
5. [Best Practices](#best-practices)

## Introduction

The `atom::io` namespace provides a set of functions for file compression, decompression, and ZIP file operations. These functions allow you to easily compress and decompress individual files, compress folders, and perform various operations on ZIP files.

## File Compression and Decompression

### compressFile

```cpp
auto compressFile(std::string_view file_name, std::string_view output_folder) -> bool;
```

Compresses a single file.

- `file_name`: The name (including path) of the file to be compressed.
- `output_folder`: The folder where the compressed file will be saved.
- Returns: `true` if compression is successful, `false` otherwise.

**Note**: If the file name already contains a `.gz` suffix, it will not be compressed again.

### decompressFile

```cpp
auto decompressFile(std::string_view file_name, std::string_view output_folder) -> bool;
```

Decompresses a single file.

- `file_name`: The name (including path) of the file to be decompressed.
- `output_folder`: The folder where the decompressed file will be saved.
- Returns: `true` if decompression is successful, `false` otherwise.

**Note**: If the file name does not contain a `.gz` suffix, it will not be decompressed.

### compressFolder

```cpp
auto compressFolder(const char *folder_name) -> bool;
```

Compresses all files in a specified directory.

- `folder_name`: The name (absolute path) of the folder to be compressed.
- Returns: `true` if compression is successful, `false` otherwise.

**Note**: The compressed files will be saved in the original directory, and files in subdirectories will not be compressed.

## ZIP File Operations

### extractZip

```cpp
auto extractZip(std::string_view zip_file, std::string_view destination_folder) -> bool;
```

Extracts a single ZIP file.

- `zip_file`: The name (including path) of the ZIP file to be extracted.
- `destination_folder`: The path where the extracted files will be saved.
- Returns: `true` if extraction is successful, `false` otherwise.

**Note**: If the specified path does not exist, the function will attempt to create it.

### createZip

```cpp
auto createZip(std::string_view source_folder, std::string_view zip_file, int compression_level = -1) -> bool;
```

Creates a ZIP file from a folder.

- `source_folder`: The name (including path) of the folder to be compressed.
- `zip_file`: The name (including path) of the resulting ZIP file.
- `compression_level`: Compression level (optional, default is -1, meaning use default level).
- Returns: `true` if creation is successful, `false` otherwise.

### listFilesInZip

```cpp
auto listFilesInZip(std::string_view zip_file) -> std::vector<std::string>;
```

Lists files in a ZIP file.

- `zip_file`: The name (including path) of the ZIP file.
- Returns: A vector of file names contained in the ZIP file.

### fileExistsInZip

```cpp
auto fileExistsInZip(std::string_view zip_file, std::string_view file_name) -> bool;
```

Checks if a specified file exists in a ZIP file.

- `zip_file`: The name (including path) of the ZIP file.
- `file_name`: The name of the file to check.
- Returns: `true` if the file exists in the ZIP, `false` otherwise.

### removeFileFromZip

```cpp
auto removeFileFromZip(std::string_view zip_file, std::string_view file_name) -> bool;
```

Removes a specified file from a ZIP file.

- `zip_file`: The name (including path) of the ZIP file.
- `file_name`: The name of the file to be removed.
- Returns: `true` if removal is successful, `false` otherwise.

### getZipFileSize

```cpp
auto getZipFileSize(std::string_view zip_file) -> size_t;
```

Gets the size of a ZIP file.

- `zip_file`: The name (including path) of the ZIP file.
- Returns: The size of the ZIP file in bytes.

## Usage Examples

Here are some examples demonstrating how to use these functions:

### Compressing and Decompressing Files

```cpp
#include "compress.hpp"
#include <iostream>

int main() {
    // Compress a file
    if (atom::io::compressFile("example.txt", "/output/folder")) {
        std::cout << "File compressed successfully." << std::endl;
    } else {
        std::cout << "Failed to compress file." << std::endl;
    }

    // Decompress a file
    if (atom::io::decompressFile("/output/folder/example.txt.gz", "/decompressed/folder")) {
        std::cout << "File decompressed successfully." << std::endl;
    } else {
        std::cout << "Failed to decompress file." << std::endl;
    }

    // Compress a folder
    if (atom::io::compressFolder("/path/to/folder")) {
        std::cout << "Folder compressed successfully." << std::endl;
    } else {
        std::cout << "Failed to compress folder." << std::endl;
    }

    return 0;
}
```

### Working with ZIP Files

```cpp
#include "compress.hpp"
#include <iostream>

int main() {
    // Create a ZIP file
    if (atom::io::createZip("/source/folder", "archive.zip")) {
        std::cout << "ZIP file created successfully." << std::endl;
    } else {
        std::cout << "Failed to create ZIP file." << std::endl;
    }

    // List files in a ZIP file
    auto files = atom::io::listFilesInZip("archive.zip");
    std::cout << "Files in ZIP:" << std::endl;
    for (const auto& file : files) {
        std::cout << "- " << file << std::endl;
    }

    // Check if a file exists in the ZIP
    if (atom::io::fileExistsInZip("archive.zip", "example.txt")) {
        std::cout << "File exists in ZIP." << std::endl;
    } else {
        std::cout << "File does not exist in ZIP." << std::endl;
    }

    // Remove a file from the ZIP
    if (atom::io::removeFileFromZip("archive.zip", "example.txt")) {
        std::cout << "File removed from ZIP successfully." << std::endl;
    } else {
        std::cout << "Failed to remove file from ZIP." << std::endl;
    }

    // Get ZIP file size
    size_t size = atom::io::getZipFileSize("archive.zip");
    std::cout << "ZIP file size: " << size << " bytes" << std::endl;

    // Extract// Extract ZIP file
    if (atom::io::extractZip("archive.zip", "/extracted/folder")) {
        std::cout << "ZIP file extracted successfully." << std::endl;
    } else {
        std::cout << "Failed to extract ZIP file." << std::endl;
    }

    return 0;
}
```

## Best Practices

When working with the compression and ZIP functions provided by the `atom::io` namespace, consider the following best practices:

1. **Error Handling**: Always check the return values of functions to ensure operations were successful. Handle potential failures gracefully in your application.

2. **Path Handling**: Use absolute paths when possible to avoid ambiguity, especially when working with different directories.

3. **Large Files**: When working with large files, consider using asynchronous I/O operations or processing in chunks to avoid blocking the main thread.

4. **Compression Levels**: When creating ZIP files, experiment with different compression levels to find the right balance between file size and compression time for your specific use case.

5. **File Naming**: Be cautious with file names, especially when compressing or extracting files. Ensure that the resulting file names are valid and don't overwrite existing files unintentionally.

6. **Security**: When extracting ZIP files, be aware of potential security risks such as path traversal attacks. Validate and sanitize file paths before extraction.

7. **Resource Management**: Close any open file handles or resources after use, especially when working with large numbers of files or in long-running applications.

## Advanced Examples

### Recursive Folder Compression

While the provided `compressFolder` function doesn't compress subdirectories, you can create a recursive version:

```cpp
#include "compress.hpp"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

bool compressFolderRecursive(const fs::path& folder_path) {
    for (const auto& entry : fs::recursive_directory_iterator(folder_path)) {
        if (fs::is_regular_file(entry)) {
            std::string relative_path = fs::relative(entry.path(), folder_path).string();
            if (!atom::io::compressFile(entry.path().string(), folder_path.string())) {
                std::cerr << "Failed to compress: " << relative_path << std::endl;
                return false;
            }
        }
    }
    return true;
}

int main() {
    if (compressFolderRecursive("/path/to/folder")) {
        std::cout << "Folder and subfolders compressed successfully." << std::endl;
    } else {
        std::cout << "Failed to compress folder and subfolders." << std::endl;
    }
    return 0;
}
```

### Batch Processing with Progress Reporting

Here's an example of how you might process multiple files with progress reporting:

```cpp
#include "compress.hpp"
#include <iostream>
#include <vector>
#include <string>

void compressFiles(const std::vector<std::string>& files, const std::string& output_folder) {
    int total_files = files.size();
    int processed_files = 0;

    for (const auto& file : files) {
        if (atom::io::compressFile(file, output_folder)) {
            processed_files++;
            float progress = (static_cast<float>(processed_files) / total_files) * 100;
            std::cout << "\rProgress: " << progress << "% (" << processed_files << "/" << total_files << ")" << std::flush;
        } else {
            std::cerr << "\nFailed to compress: " << file << std::endl;
        }
    }
    std::cout << "\nCompression completed." << std::endl;
}

int main() {
    std::vector<std::string> files_to_compress = {
        "/path/to/file1.txt",
        "/path/to/file2.txt",
        "/path/to/file3.txt"
    };
    compressFiles(files_to_compress, "/output/folder");
    return 0;
}
```

### Creating a ZIP File with Password Protection

While the current `createZip` function doesn't support password protection, you could extend it to include this feature. Here's a conceptual example of how you might use such a function:

```cpp
#include "compress.hpp"
#include <iostream>

int main() {
    std::string source_folder = "/path/to/folder";
    std::string zip_file = "protected_archive.zip";
    std::string password = "your_secure_password";

    // Note: This is a hypothetical function not provided in the original interface
    if (atom::io::createProtectedZip(source_folder, zip_file, password)) {
        std::cout << "Password-protected ZIP file created successfully." << std::endl;
    } else {
        std::cout << "Failed to create password-protected ZIP file." << std::endl;
    }

    return 0;
}
```

## Conclusion

The compression and ZIP operations provided by the `atom::io` namespace offer a convenient way to work with compressed files and ZIP archives in C++. By following the best practices and examples provided in this documentation, you can effectively integrate these functions into your projects, handling file compression, decompression, and ZIP operations with ease.

Remember to always handle errors, manage resources properly, and consider the specific requirements of your application when using these functions. With proper use, these tools can significantly enhance your file handling capabilities in C++ applications.
