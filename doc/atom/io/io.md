# atom::io Namespace Documentation

This document provides a detailed explanation of the functions and structures in the `atom::io` namespace, which is defined in the `io.hpp` header file. The namespace contains various utility functions for file and directory operations.

## Table of Contents

1. [Directory Operations](#directory-operations)
2. [File Operations](#file-operations)
3. [Symlink Operations](#symlink-operations)
4. [Path and Name Validation](#path-and-name-validation)
5. [File Type and Size Operations](#file-type-and-size-operations)
6. [File Splitting and Merging](#file-splitting-and-merging)
7. [Miscellaneous Functions](#miscellaneous-functions)

## Directory Operations

### createDirectory

```cpp
[[nodiscard]] auto createDirectory(const std::string &path) -> bool;
```

Creates a directory with the specified path.

- **Parameters:**
  - `path`: The path of the directory to create.
- **Returns:** `true` if the operation was successful, `false` otherwise.

**Example:**

```cpp
if (atom::io::createDirectory("/path/to/new/directory")) {
    std::cout << "Directory created successfully." << std::endl;
} else {
    std::cout << "Failed to create directory." << std::endl;
}
```

### createDirectoriesRecursive

```cpp
auto createDirectoriesRecursive(
    const fs::path &basePath, const std::vector<std::string> &subdirs,
    const CreateDirectoriesOptions &options) -> bool;
```

Creates a directory structure recursively with the specified base path and subdirectories.

- **Parameters:**
  - `basePath`: The base path of the directory structure.
  - `subdirs`: A vector of subdirectory names to create.
  - `options`: A `CreateDirectoriesOptions` struct with additional options.
- **Returns:** `true` if the operation was successful, `false` otherwise.

**Example:**

```cpp
atom::io::CreateDirectoriesOptions options;
options.verbose = true;
options.dryRun = false;
options.delay = 100; // milliseconds

std::vector<std::string> subdirs = {"folder1", "folder2/subfolder", "folder3"};

if (atom::io::createDirectoriesRecursive("/path/to/base", subdirs, options)) {
    std::cout << "Directories created successfully." << std::endl;
} else {
    std::cout << "Failed to create directories." << std::endl;
}
```

### removeDirectory

```cpp
[[nodiscard]] auto removeDirectory(const std::string &path) -> bool;
```

Removes an empty directory with the specified path.

- **Parameters:**
  - `path`: The path of the directory to remove.
- **Returns:** `true` if the operation was successful, `false` otherwise.

**Example:**

```cpp
if (atom::io::removeDirectory("/path/to/empty/directory")) {
    std::cout << "Directory removed successfully." << std::endl;
} else {
    std::cout << "Failed to remove directory." << std::endl;
}
```

### removeDirectoriesRecursive

```cpp
[[nodiscard]] auto removeDirectoriesRecursive(
    const fs::path &basePath, const std::vector<std::string> &subdirs,
    const CreateDirectoriesOptions &options = {}) -> bool;
```

Removes a directory structure recursively with the specified base path and subdirectories.

- **Parameters:**
  - `basePath`: The base path of the directory structure.
  - `subdirs`: A vector of subdirectory names to remove.
  - `options`: A `CreateDirectoriesOptions` struct with additional options (optional).
- **Returns:** `true` if the operation was successful, `false` otherwise.

**Example:**

```cpp
std::vector<std::string> subdirs = {"folder1", "folder2/subfolder", "folder3"};

if (atom::io::removeDirectoriesRecursive("/path/to/base", subdirs)) {
    std::cout << "Directories removed successfully." << std::endl;
} else {
    std::cout << "Failed to remove directories." << std::endl;
}
```

### renameDirectory

```cpp
[[nodiscard]] auto renameDirectory(const std::string &old_path,
                                   const std::string &new_path) -> bool;
```

Renames a directory from the old path to the new path.

- **Parameters:**
  - `old_path`: The current path of the directory.
  - `new_path`: The new path for the directory.
- **Returns:** `true` if the operation was successful, `false` otherwise.

**Example:**

```cpp
if (atom::io::renameDirectory("/path/to/old_dir", "/path/to/new_dir")) {
    std::cout << "Directory renamed successfully." << std::endl;
} else {
    std::cout << "Failed to rename directory." << std::endl;
}
```

### moveDirectory

```cpp
[[nodiscard]] auto moveDirectory(const std::string &old_path,
                                 const std::string &new_path) -> bool;
```

Moves a directory from the old path to the new path.

- **Parameters:**
  - `old_path`: The current path of the directory.
  - `new_path`: The new path for the directory.
- **Returns:** `true` if the operation was successful, `false` otherwise.

**Example:**

```cpp
if (atom::io::moveDirectory("/path/to/source_dir", "/path/to/destination_dir")) {
    std::cout << "Directory moved successfully." << std::endl;
} else {
    std::cout << "Failed to move directory." << std::endl;
}
```

## File Operations

### copyFile

```cpp
[[nodiscard]] auto copyFile(const std::string &src_path,
                            const std::string &dst_path) -> bool;
```

Copies a file from the source path to the destination path.

- **Parameters:**
  - `src_path`: The path of the source file.
  - `dst_path`: The path of the destination file.
- **Returns:** `true` if the operation was successful, `false` otherwise.

**Example:**

```cpp
if (atom::io::copyFile("/path/to/source.txt", "/path/to/destination.txt")) {
    std::cout << "File copied successfully." << std::endl;
} else {
    std::cout << "Failed to copy file." << std::endl;
}
```

### moveFile

```cpp
[[nodiscard]] auto moveFile(const std::string &src_path,
                            const std::string &dst_path) -> bool;
```

Moves a file from the source path to the destination path.

- **Parameters:**
  - `src_path`: The current path of the file.
  - `dst_path`: The new path for the file.
- **Returns:** `true` if the operation was successful, `false` otherwise.

**Example:**

```cpp
if (atom::io::moveFile("/path/to/source.txt", "/path/to/destination.txt")) {
    std::cout << "File moved successfully." << std::endl;
} else {
    std::cout << "Failed to move file." << std::endl;
}
```

### renameFile

```cpp
[[nodiscard]] auto renameFile(const std::string &old_path,
                              const std::string &new_path) -> bool;
```

Renames a file from the old path to the new path.

- **Parameters:**
  - `old_path`: The current path of the file.
  - `new_path`: The new path for the file.
- **Returns:** `true` if the operation was successful, `false` otherwise.

**Example:**

```cpp
if (atom::io::renameFile("/pathto/oldname.txt", "/path/to/newname.txt")) {
    std::cout << "File renamed successfully." << std::endl;
} else {
    std::cout << "Failed to rename file." << std::endl;
}
```

### removeFile

```cpp
[[nodiscard]] auto removeFile(const std::string &path) -> bool;
```

Removes a file with the specified path.

- **Parameters:**
  - `path`: The path of the file to remove.
- **Returns:** `true` if the operation was successful, `false` otherwise.

**Example:**

```cpp
if (atom::io::removeFile("/path/to/file.txt")) {
    std::cout << "File removed successfully." << std::endl;
} else {
    std::cout << "Failed to remove file." << std::endl;
}
```

### truncateFile

```cpp
auto truncateFile(const std::string &path, std::streamsize size) -> bool;
```

Truncates a file to a specified size.

- **Parameters:**
  - `path`: The path of the file to truncate.
  - `size`: The size to truncate the file to.
- **Returns:** `true` if the operation was successful, `false` otherwise.

**Example:**

```cpp
if (atom::io::truncateFile("/path/to/file.txt", 1024)) {
    std::cout << "File truncated successfully." << std::endl;
} else {
    std::cout << "Failed to truncate file." << std::endl;
}
```

## Symlink Operations

### createSymlink

```cpp
[[nodiscard]] auto createSymlink(const std::string &target_path,
                                 const std::string &symlink_path) -> bool;
```

Creates a symbolic link with the specified target and symlink paths.

- **Parameters:**
  - `target_path`: The path of the target file or directory for the symlink.
  - `symlink_path`: The path of the symlink to create.
- **Returns:** `true` if the operation was successful, `false` otherwise.

**Example:**

```cpp
if (atom::io::createSymlink("/path/to/target", "/path/to/symlink")) {
    std::cout << "Symlink created successfully." << std::endl;
} else {
    std::cout << "Failed to create symlink." << std::endl;
}
```

### removeSymlink

```cpp
[[nodiscard]] auto removeSymlink(const std::string &path) -> bool;
```

Removes a symbolic link with the specified path.

- **Parameters:**
  - `path`: The path of the symlink to remove.
- **Returns:** `true` if the operation was successful, `false` otherwise.

**Example:**

```cpp
if (atom::io::removeSymlink("/path/to/symlink")) {
    std::cout << "Symlink removed successfully." << std::endl;
} else {
    std::cout << "Failed to remove symlink." << std::endl;
}
```

## Path and Name Validation

### isFolderNameValid

```cpp
[[nodiscard]] auto isFolderNameValid(const std::string &folderName) -> bool;
```

Checks if the folder name is valid.

- **Parameters:**
  - `folderName`: The folder name to check.
- **Returns:** `true` if the folder name is valid, `false` otherwise.

**Example:**

```cpp
if (atom::io::isFolderNameValid("my_folder")) {
    std::cout << "Folder name is valid." << std::endl;
} else {
    std::cout << "Folder name is invalid." << std::endl;
}
```

### isFileNameValid

```cpp
[[nodiscard]] auto isFileNameValid(const std::string &fileName) -> bool;
```

Checks if the file name is valid.

- **Parameters:**
  - `fileName`: The file name to check.
- **Returns:** `true` if the file name is valid, `false` otherwise.

**Example:**

```cpp
if (atom::io::isFileNameValid("document.txt")) {
    std::cout << "File name is valid." << std::endl;
} else {
    std::cout << "File name is invalid." << std::endl;
}
```

### isAbsolutePath

```cpp
[[nodiscard]] auto isAbsolutePath(const std::string &path) -> bool;
```

Checks if the path is an absolute path.

- **Parameters:**
  - `path`: The path to check.
- **Returns:** `true` if the path is an absolute path, `false` otherwise.

**Example:**

```cpp
if (atom::io::isAbsolutePath("/home/user/document.txt")) {
    std::cout << "Path is absolute." << std::endl;
} else {
    std::cout << "Path is relative." << std::endl;
}
```

## File Type and Size Operations

### fileSize

```cpp
[[nodiscard]] auto fileSize(const std::string &path) -> std::uintmax_t;
```

Returns the size of a file in bytes.

- **Parameters:**
  - `path`: The path of the file to get the size of.
- **Returns:** The size of the file in bytes, or 0 if the file does not exist or cannot be read.

**Example:**

```cpp
std::uintmax_t size = atom::io::fileSize("/path/to/file.txt");
std::cout << "File size: " << size << " bytes" << std::endl;
```

### checkFileTypeInFolder

```cpp
[[nodiscard]] auto checkFileTypeInFolder(
    const std::string &folderPath, const std::string &fileType,
    FileOption fileOption) -> std::vector<std::string>;
```

Checks the file type in the folder and returns a vector of file paths or names.

- **Parameters:**
  - `folderPath`: The folder path to check.
  - `fileType`: The file type to check (e.g., ".txt", ".cpp").
  - `fileOption`: An enum class `FileOption` specifying whether to return the full path (`PATH`) or just the file name (`NAME`).
- **Returns:** A vector of file paths or names matching the specified file type.

**Example:**

```cpp
std::vector<std::string> textFiles = atom::io::checkFileTypeInFolder("/path/to/folder", ".txt", atom::io::FileOption::PATH);
for (const auto& file : textFiles) {
    std::cout << "Found text file: " << file << std::endl;
}
```

## File Splitting and Merging

### splitFile

```cpp
void splitFile(const std::string &filePath, std::size_t chunkSize,
               const std::string &outputPattern = "");
```

Splits a file into multiple parts.

- **Parameters:**
  - `filePath`: The file path of the file to split.
  - `chunkSize`: The size of each chunk in bytes.
  - `outputPattern`: The output file pattern (optional).

**Example:**

```cpp
atom::io::splitFile("/path/to/largefile.dat", 1024 * 1024, "output_part_");
```

### mergeFiles

```cpp
void mergeFiles(const std::string &outputFilePath,
                const std::vector<std::string> &partFiles);
```

Merges multiple parts into a single file.

- **Parameters:**
  - `outputFilePath`: The output file path for the merged file.
  - `partFiles`: A vector of file paths for the parts to merge.

**Example:**

```cpp
std::vector<std::string> partFiles = {"part1.dat", "part2.dat", "part3.dat"};
atom::io::mergeFiles("/path/to/merged_file.dat", partFiles);
```

### quickSplit

```cpp
void quickSplit(const std::string &filePath, int numChunks,
                const std::string &outputPattern = "");
```

Quicklysplits a file into multiple parts.

- **Parameters:**
  - `filePath`: The file path of the file to split.
  - `numChunks`: The number of chunks to split the file into.
  - `outputPattern`: The output file pattern (optional).

**Example:**

```cpp
atom::io::quickSplit("/path/to/largefile.dat", 5, "output_chunk_");
```

### quickMerge

```cpp
void quickMerge(const std::string &outputFilePath,
                const std::string &partPattern, int numChunks);
```

Quickly merges multiple parts into a single file.

- **Parameters:**
  - `outputFilePath`: The output file path for the merged file.
  - `partPattern`: The pattern of the part files to merge.
  - `numChunks`: The number of chunks to merge.

**Example:**

```cpp
atom::io::quickMerge("/path/to/merged_file.dat", "output_chunk_", 5);
```

## Miscellaneous Functions

### jwalk

```cpp
auto jwalk(const std::string &root) -> std::string;
```

Recursively walks through a directory and its subdirectories, returning a JSON string containing file information.

- **Parameters:**
  - `root`: The root path of the directory to walk.
- **Returns:** A JSON string containing the file information.

**Example:**

```cpp
std::string fileInfo = atom::io::jwalk("/path/to/directory");
std::cout << "Directory structure: " << fileInfo << std::endl;
```

### fwalk

```cpp
void fwalk(const fs::path &root,
           const std::function<void(const fs::path &)> &callback);
```

Recursively walks through a directory and its subdirectories, applying a callback function to each file.

- **Parameters:**
  - `root`: The root path of the directory to walk.
  - `callback`: The callback function to execute for each file.

**Example:**

```cpp
atom::io::fwalk("/path/to/directory", [](const fs::path& filePath) {
    std::cout << "Found file: " << filePath << std::endl;
});
```

### convertToLinuxPath

```cpp
[[nodiscard]] auto convertToLinuxPath(std::string_view windows_path)
    -> std::string;
```

Converts a Windows path to a Linux path by replacing backslashes with forward slashes.

- **Parameters:**
  - `windows_path`: The Windows path to convert.
- **Returns:** The converted Linux path.

**Example:**

```cpp
std::string linuxPath = atom::io::convertToLinuxPath("C:\\Users\\John\\Documents");
std::cout << "Linux path: " << linuxPath << std::endl;
```

### convertToWindowsPath

```cpp
[[nodiscard]] auto convertToWindowsPath(std::string_view linux_path)
    -> std::string;
```

Converts a Linux path to a Windows path by replacing forward slashes with backslashes.

- **Parameters:**
  - `linux_path`: The Linux path to convert.
- **Returns:** The converted Windows path.

**Example:**

```cpp
std::string windowsPath = atom::io::convertToWindowsPath("/home/john/documents");
std::cout << "Windows path: " << windowsPath << std::endl;
```

### normPath

```cpp
[[nodiscard]] auto normPath(std::string_view raw_path) -> std::string;
```

Normalizes a path by removing redundant separators and resolving ".." and "." components.

- **Parameters:**
  - `raw_path`: The path to normalize.
- **Returns:** The normalized path.

**Example:**

```cpp
std::string normalizedPath = atom::io::normPath("/home/user/../john/./documents");
std::cout << "Normalized path: " << normalizedPath << std::endl;
```

### isFolderExists

```cpp
[[nodiscard]] auto isFolderExists(const std::string &folderName) -> bool;
```

Checks if the folder exists.

- **Parameters:**
  - `folderName`: The folder path to check.
- **Returns:** `true` if the folder exists, `false` otherwise.

**Example:**

```cpp
if (atom::io::isFolderExists("/path/to/folder")) {
    std::cout << "Folder exists." << std::endl;
} else {
    std::cout << "Folder does not exist." << std::endl;
}
```

### isFileExists

```cpp
[[nodiscard]] auto isFileExists(const std::string &fileName) -> bool;
```

Checks if the file exists.

- **Parameters:**
  - `fileName`: The file path to check.
- **Returns:** `true` if the file exists, `false` otherwise.

**Example:**

```cpp
if (atom::io::isFileExists("/path/to/file.txt")) {
    std::cout << "File exists." << std::endl;
} else {
    std::cout << "File does not exist." << std::endl;
}
```

### isFolderEmpty

```cpp
[[nodiscard]] auto isFolderEmpty(const std::string &folderName) -> bool;
```

Checks if the folder is empty.

- **Parameters:**
  - `folderName`: The folder path to check.
- **Returns:** `true` if the folder is empty, `false` otherwise.

**Example:**

```cpp
if (atom::io::isFolderEmpty("/path/to/folder")) {
    std::cout << "Folder is empty." << std::endl;
} else {
    std::cout << "Folder is not empty." << std::endl;
}
```

### changeWorkingDirectory

```cpp
[[nodiscard]] auto changeWorkingDirectory(const std::string &directoryPath)
    -> bool;
```

Changes the working directory.

- **Parameters:**
  - `directoryPath`: The directory path to change to.
- **Returns:** `true` if the working directory was changed successfully, `false` otherwise.

**Example:**

```cpp
if (atom::io::changeWorkingDirectory("/path/to/new/working/directory")) {
    std::cout << "Working directory changed successfully." << std::endl;
} else {
    std::cout << "Failed to change working directory." << std::endl;
}
```

### getFileTimes

```cpp
[[nodiscard]] std::pair<std::string, std::string> getFileTimes(
    const std::string &filePath);
```

Gets the creation and modification times of a file.

- **Parameters:**
  - `filePath`: The path of the file.
- **Returns:** A pair of strings containing the creation time and modification time.

**Example:**

```cpp
auto [creationTime, modificationTime] = atom::io::getFileTimes("/path/to/file.txt");
std::cout << "Creation time: " << creationTime << std::endl;
std::cout << "Modification time: " << modificationTime << std::endl;
```

### isExecutableFile

```cpp
auto isExecutableFile(const std::string &fileName,
                      const std::string &fileExt) -> bool;
```

Checks whether the specified file exists and is executable.

- **Parameters:**
  - `fileName`: The name of the file.
  - `fileExt`: The extension of the file.
- **Returns:** `true` if the file exists and is executable, `false` otherwise.

**Example:**

```cpp
if (atom::io::isExecutableFile("myprogram", ".exe")) {
    std::cout << "File is executable." << std::endl;
} else {
    std::cout << "File is not executable or does not exist." << std::endl;
}
```

### getFileSize

```cpp
auto getFileSize(const std::string &filePath) -> std::size_t;
```

Gets the file size.

- **Parameters:**
  - `filePath`: The file path.
- **Returns:** The file size in bytes.

**Example:**

```cpp
std::size_t size = atom::io::getFileSize("/path/to/file.txt");
std::cout << "File size: " << size << " bytes" << std::endl;
```

### calculateChunkSize

```cpp
auto calculateChunkSize(std::size_t fileSize, int numChunks) -> std::size_t;
```

Calculates the chunk size for file splitting.

- **Parameters:**
  - `fileSize`: The total file size.
  - `numChunks`: The number of chunks to split the file into.
- **Returns:** The calculated chunk size.

**Example:**

```cpp
std::size_t fileSize = 1024 * 1024 * 10; // 10 MB
int numChunks = 4;
std::size_t chunkSize = atom::io::calculateChunkSize(fileSize, numChunks);
std::cout << "Chunk size: " << chunkSize << " bytes" << std::endl;
```

### getExecutableNameFromPath

```cpp
[[nodiscard]]
auto getExecutableNameFromPath(const std::string &path) -> std::string;
```

Gets the executable name from the path.

- **Parameters:**
  - `path`: The path of the executable.
- **Returns:** The executable name.

**Example:**

```cpp
std::string exeName = atom::io::getExecutableNameFromPath("/usr/bin/myprogram");
std::cout << "Executable name: " << exeName << std::endl;
```

## Structures and Enums

### CreateDirectoriesOptions

```cpp
struct CreateDirectoriesOptions {
    bool verbose = true;
    bool dryRun = false;
    int delay = 0;
    std::function<bool(const std::string &)> filter = [](const std::string &) {
        return true;
    };
    std::function<void(const std::string &)> onCreate =
        [](const std::string &) {};
    std::function<void(const std::string &)> onDelete =
        [](const std::string &) {};
};
```

This structure provides options for directory creation operations.

- `verbose`: If true, enables verbose output.
- `dryRun`: If true, performs a dry run without actually creating directories.
- `delay`: Delay in milliseconds between operations.
- `filter`: A function that filters which directories should be created.
- `onCreate`: A function called when a directory is created.
- `onDelete`: A function called when a directory is deleted.

### FileOption

```cpp
enum class FileOption { PATH, NAME };
```

This enum class is used in the `checkFileTypeInFolder` function to specify whether to return full paths or just file names.

- `PATH`: Return full file paths.
- `NAME`: Return only file names.

## Best Practices and Notes

1. Always check the return values of functions that return a boolean to ensure the operation was successful.

2. Use the `[[nodiscard]]` attribute on functions that return important values to encourage checking the result.

3. When working with file paths, consider using the `normPath` function to normalize paths for consistency.

4. For file operations that might take a long time (like splitting or merging large files), consider implementing progress reporting or using asynchronous operations in your application.

5. Be cautious when using functions that modify the file system (like delete or move operations). Always double-check paths and consider using dry-run options when available.

6. When working with symlinks, be aware of the potential security implications, especially when dealing with user-provided paths.

7. For cross-platform applications, use the path conversion functions (`convertToLinuxPath` and `convertToWindowsPath`) to ensure path compatibility.

8. When implementing file type checks or operations based on file extensions, remember that file extensions are not always a reliable indicator of file content.

9. Consider using the `CreateDirectoriesOptions` structure to customize directory creation behavior, especially for more complex scenarios.

10. When splitting or merging files, choose between the regular and "quick" versions based on your specific requirements for speed vs. flexibility.

By following these practices and using the `atom::io` namespace functions effectively, you can create robust and efficient file and directory management systems in your C++ applications.
