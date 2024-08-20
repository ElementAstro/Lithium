/*
 * compress.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-3-31

Description: Compressor using ZLib

**************************************************/

#ifndef ATOM_IO_COMPRESS_HPP
#define ATOM_IO_COMPRESS_HPP

#include <string>
#include <vector>

namespace atom::io {
/**
 * @brief Compress a single file
 * @param file_name The name (including path) of the file to be compressed.
 * @param output_folder The folder where the compressed file will be saved.
 * @return Whether the compression is successful.
 *
 * This function compresses a single file, and the compressed file is named by
 * adding the .gz suffix to the source file name.
 *
 * @note If the file name already contains a .gz suffix, it will not be
 * compressed again.
 */
auto compressFile(std::string_view file_name,
                  std::string_view output_folder) -> bool;

/**
 * @brief Decompress a single file
 * @param file_name The name (including path) of the file to be decompressed.
 * @param output_folder The folder where the decompressed file will be saved.
 * @return Whether the decompression is successful.
 *
 * This function decompresses a single compressed file, and the decompressed
 * file is named by removing the .gz suffix from the source file name.
 *
 * @note If the file name does not contain a .gz suffix, it will not be
 * decompressed.
 */
auto decompressFile(std::string_view file_name,
                    std::string_view output_folder) -> bool;

/**
 * @brief Compress all files in a specified directory
 * @param folder_name The name (absolute path) of the folder to be compressed.
 * @return Whether the compression is successful.
 *
 * This function compresses all files in the specified folder, and the
 * compressed file is named by adding the .gz suffix to each source file name.
 *
 * @note The compressed files will be saved in the original directory, and files
 * in subdirectories will not be compressed.
 */
auto compressFolder(const char *folder_name) -> bool;

/**
 * @brief Extract a single ZIP file
 * @param zip_file The name (including path) of the ZIP file to be extracted.
 * @param destination_folder The path where the extracted files will be saved
 * (including path).
 * @return Whether the extraction is successful.
 *
 * This function extracts a single ZIP file, and the extracted files are saved
 * in the specified path.
 *
 * @note If the specified path does not exist, the function will attempt to
 * create it.
 */
auto extractZip(std::string_view zip_file,
                std::string_view destination_folder) -> bool;

/**
 * @brief Create a ZIP file
 * @param source_folder The name (including path) of the folder to be
 * compressed.
 * @param zip_file The name (including path) of the resulting ZIP file.
 * @param compression_level Compression level (optional, default is -1, meaning
 * use default level).
 * @return Whether the creation is successful.
 *
 * This function creates a ZIP file and compresses the files in the specified
 * folder into the ZIP file.
 *
 * @note If the specified path does not exist, the function will attempt to
 * create it.
 */
auto createZip(std::string_view source_folder, std::string_view zip_file,
               int compression_level = -1) -> bool;

/**
 * @brief List files in a ZIP file
 * @param zip_file The name (including path) of the ZIP file.
 * @return A list of file names.
 *
 * This function lists the files in a ZIP file.
 *
 * @note If the specified ZIP file does not exist, the function will return an
 * empty list.
 */
auto listFilesInZip(std::string_view zip_file) -> std::vector<std::string>;

/**
 * @brief Check if a specified file exists in a ZIP file
 * @param zip_file The name (including path) of the ZIP file.
 * @param file_name The name of the file to check.
 * @return Whether the file exists.
 *
 * This function checks if a specified file exists in a ZIP file.
 *
 * @note If the specified ZIP file does not exist, the function will return
 * false.
 */
auto fileExistsInZip(std::string_view zip_file,
                     std::string_view file_name) -> bool;

/**
 * @brief Remove a specified file from a ZIP file
 * @param zip_file The name (including path) of the ZIP file.
 * @param file_name The name of the file to be removed.
 * @return Whether the removal is successful.
 *
 * This function removes a specified file from a ZIP file.
 *
 * @note If the specified ZIP file does not exist, the function will return
 * false.
 */
auto removeFileFromZip(std::string_view zip_file,
                       std::string_view file_name) -> bool;

/**
 * @brief Get the size of a file in a ZIP file
 * @param zip_file The name (including path) of the ZIP file.
 * @return The file size.
 *
 * This function gets the size of a file in a ZIP file.
 *
 * @note If the specified ZIP file does not exist, the function will return 0.
 */
auto getZipFileSize(std::string_view zip_file) -> size_t;
}  // namespace atom::io

#endif
