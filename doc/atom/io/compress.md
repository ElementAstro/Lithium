# File Compression and Decompression Functions

## compress_file

Compresses a single file by adding the .gz suffix to the file name.

```cpp
bool success = compress_file("/path/to/file.txt", "/output/folder");
```

## decompress_file

Decompresses a single compressed file by removing the .gz suffix from the file name.

```cpp
bool success = decompress_file("/path/to/compressed_file.gz", "/output/folder");
```

## compress_folder

Compresses all files in a specified folder by adding the .gz suffix to each file.

```cpp
bool success = compress_folder("/absolute/path/to/folder");
```

## create_zip

Creates a ZIP file containing the contents of a specified folder.

```cpp
bool success = create_zip("/source/folder", "/output/archive.zip");
```

## extract_zip

Extracts the contents of a ZIP file to a specified destination folder.

```cpp
bool success = extract_zip("/source/archive.zip", "/destination/folder");
```
