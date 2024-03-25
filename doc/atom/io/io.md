# File and Directory Manipulation Functions

## createDirectory

Creates a directory with the specified path.

```cpp
bool success = createDirectory("/path/to/directory");
```

## createDirectory (overload)

Creates a directory with the specified date and root directory.

```cpp
createDirectory("2024-03-25", "/root/directory");
```

## removeDirectory

Removes an empty directory with the specified path.

```cpp
bool success = removeDirectory("/path/to/directory");
```

## renameDirectory

Renames a directory with the specified old and new paths.

```cpp
bool success = renameDirectory("/old/path", "/new/path");
```

## moveDirectory

Moves a directory from one path to another.

```cpp
bool success = moveDirectory("/old/path", "/new/path");
```

## copyFile

Copies a file from source path to destination path.

```cpp
bool success = copyFile("/source/file.txt", "/destination/file.txt");
```

## moveFile

Moves a file from source path to destination path.

```cpp
bool success = moveFile("/source/file.txt", "/destination/file.txt");
```

## renameFile

Renames a file with the specified old and new paths.

```cpp
bool success = renameFile("/old/file.txt", "/new/file.txt");
```

## removeFile

Removes a file with the specified path.

```cpp
bool success = removeFile("/path/to/file.txt");
```

## createSymlink

Creates a symbolic link with the specified target and symlink paths.

```cpp
bool success = createSymlink("/target/file.txt", "/symlink/file.txt");
```

## removeSymlink

Removes a symbolic link with the specified path.

```cpp
bool success = removeSymlink("/path/to/symlink");
```

## fileSize

Returns the size of a file in bytes.

```cpp
std::uintmax_t size = fileSize("/path/to/file.txt");
```

## traverseDirectories

Traverse the directories recursively and collect all folder paths.

```cpp
std::vector<std::string> folders;
traverseDirectories("/start/directory", folders);
```

## convertToLinuxPath

Convert Windows path to Linux path.

```cpp
std::string linuxPath = convertToLinuxPath("C:\\Windows\\file.txt");
```

## convertToWindowsPath

Convert Linux path to Windows path.

```cpp
std::string windowsPath = convertToWindowsPath("/home/user/file.txt");
```

## isFolderNameValid

Check if the folder name is valid.

```cpp
bool valid = isFolderNameValid("folder_name");
```

## isFileNameValid

Check if the file name is valid.

```cpp
bool valid = isFileNameValid("file_name.txt");
```

## isFolderExists

Check if the folder exists.

```cpp
bool exists = isFolderExists("/path/to/folder");
```

## isFileExists

Check if the file exists.

```cpp
bool exists = isFileExists("/path/to/file.txt");
```

## isFolderEmpty

Check if the folder is empty.

```cpp
bool empty = isFolderEmpty("/path/to/folder");
```

## isAbsolutePath

Check if the path is an absolute path.

```cpp
bool absolute = isAbsolutePath("/absolute/path");
```

## normPath

Normalize a given path.

```cpp
std::string normalizedPath = normPath("/path/to/normalize");
```

## changeWorkingDirectory

Change the current working directory.

```cpp
bool success = changeWorkingDirectory("/new/work/directory");
```

## getFileTimes

Get the creation and modification times of a file.

```cpp
std::pair<std::string, std::string> times = getFileTimes("/path/to/file.txt");
```

## checkFileTypeInFolder

Check the file type in a folder.

```cpp
std::vector<std::string> filePaths = checkFileTypeInFolder("/path/to/folder", "txt", FileOption::Extension);
```
