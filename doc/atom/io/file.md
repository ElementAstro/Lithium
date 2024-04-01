# FileManager Class

## Brief

The FileManager class provides functionality for creating, opening, reading, writing, moving, and deleting files.

## Constructor

```cpp
FileManager fileManager;
```

## createFile

Creates a new file with the given filename.

```cpp
bool success = fileManager.createFile("example.txt");
```

## openFile

Opens an existing file with the specified filename.

```cpp
bool success = fileManager.openFile("example.txt");
```

## readFile

Reads the contents of the currently open file into a string variable.

```cpp
std::string contents;
bool success = fileManager.readFile(contents);
```

## writeFile

Writes the provided content to the currently open file.

```cpp
bool success = fileManager.writeFile("This is some text to write to the file.");
```

## moveFile

Moves a file from the old filename to the new filename.

```cpp
bool success = fileManager.moveFile("old_file.txt", "new_file.txt");
```

## deleteFile

Deletes the file with the given filename.

```cpp
bool success = fileManager.deleteFile("file_to_delete.txt");
```

## getFileSize

Gets the size of the currently open file in bytes.

```cpp
long fileSize = fileManager.getFileSize();
```

## calculateSHA256

Calculates the SHA256 hash of the currently open file.

```cpp
std::string hash = fileManager.calculateSHA256();
```

## getFileDirectory

Static method to get the directory path of a file.

```cpp
std::string directory = FileManager::getFileDirectory("path/to/file.txt");
```
