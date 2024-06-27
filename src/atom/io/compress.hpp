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
 * @brief 对单个文件进行压缩
 * @param file_name 待压缩的文件名（包含路径）
 * @return 是否压缩成功
 *
 * 该函数用于压缩单个文件，压缩后的文件命名方式为源文件名添加.gz后缀名。
 *
 * @note 如果文件名中已经包含.gz后缀名，则不会对其进行重复压缩。
 *
 * @param file_name The name (including path) of the file to be compressed.
 * @return Whether the compression is successful.
 *
 * This function is used to compress a single file, and the compressed file is
 * named by adding the .gz suffix to the source file name.
 *
 * @note If the file name already contains the .gz suffix, it will not be
 * compressed again.
 */
bool compress_file(const std::string &file_name,
                   const std::string &output_folder);

/**
 * @brief 对单个文件进行解压缩
 * @param file_name 待解压的文件名（包含路径）
 * @return 是否解压成功
 *
 * 该函数用于解压缩单个已经被压缩的文件，解压缩后的文件命名方式为源文件名去掉.gz后缀名。
 *
 * @note 如果文件名中没有包含.gz后缀名，则不会进行解压缩。
 *
 * @param file_name The name (including path) of the file to be decompressed.
 * @return Whether the decompression is successful.
 *
 * This function is used to decompress a single compressed file, and the
 * decompressed file is named by removing the .gz suffix from the source file
 * name.
 *
 * @note If the file name does not contain the .gz suffix, it will not be
 * decompressed.
 */
bool decompress_file(const std::string &file_name,
                     const std::string &output_folder);

/**
 * @brief 对指定目录下的文件进行压缩
 * @param folder_name 待压缩的目录名（绝对路径）
 * @return 是否压缩成功
 *
 * 该函数用于压缩指定目录下的所有文件，压缩后的文件命名方式为每个源文件名添加.gz后缀名。
 *
 * @note 压缩后的文件将保存在原目录下，并且不会对子目录中的文件进行压缩。
 *
 * @param folder_name The name (absolute path) of the folder to be compressed.
 * @return Whether the compression is successful.
 *
 * This function is used to compress all files in the specified folder, and the
 * compressed file is named by adding .gz suffix to each source file name.
 *
 * @note The compressed files will be saved in the original directory, and files
 * in subdirectories will not be compressed.
 */
bool compress_folder(const char *folder_name);

/**
 * @brief 对单个ZIP文件进行解压缩
 * @param zip_file 待解压的ZIP文件名（包含路径）
 * @param destination_folder 解压后的文件保存路径（包含路径）
 * @return 是否解压成功
 *
 * 该函数用于解压缩单个ZIP文件，解压后的文件将保存在指定的路径下。
 *
 * @note 如果指定的路径不存在，则函数将尝试创建该路径。
 *
 */
bool extract_zip(const std::string &zip_file,
                 const std::string &destination_folder);

/**
 * @brief 创建ZIP文件
 * @param source_folder 待压缩的文件夹名（包含路径）
 * @param zip_file 压缩后的ZIP文件名（包含路径）
 * @param compression_level 压缩级别（可选，默认为-1，表示使用默认级别）
 * @return 是否创建成功
 *
 * 该函数用于创建ZIP文件，并将指定文件夹中的文件压缩到ZIP文件中。
 *
 * @note如果指定的路径不存在，则函数将尝试创建该路径。
 */
bool create_zip(const std::string &source_folder, const std::string &zip_file,
                int compression_level = -1);
/**
 * @brief 列出ZIP文件中的文件列表
 * @param zip_file ZIP文件名（包含路径）
 * @return 文件列表
 *
 * 该函数用于列出ZIP文件中的文件列表。
 *
 * @note 如果指定的ZIP文件不存在，则函数将返回空列表。
 */
std::vector<std::string> list_files_in_zip(const std::string &zip_file);

/**
 * @brief 判断ZIP文件中是否存在指定的文件
 * @param zip_file ZIP文件名（包含路径）
 * @param file_name 文件名
 * @return 是否存在
 *
 * 该函数用于判断ZIP文件中是否存在指定的文件。
 *
 * @note 如果指定的ZIP文件不存在，则函数将返回false。
 */
bool file_exists_in_zip(const std::string &zip_file,
                        const std::string &file_name);

/**
 * @brief 从ZIP文件中删除指定的文件
 * @param zip_file ZIP文件名（包含路径）
 * @param file_name 文件名
 * @return 是否删除成功
 *
 * 该函数用于从ZIP文件中删除指定的文件。
 *
 * @note 如果指定的ZIP文件不存在，则函数将返回false。
 */
bool remove_file_from_zip(const std::string &zip_file,
                          const std::string &file_name);

/**
 * @brief 获取ZIP文件中的文件大小
 * @param zip_file ZIP文件名（包含路径）
 * @return 文件大小
 *
 * 该函数用于获取ZIP文件中的文件大小。
 *
 * @note 如果指定的ZIP文件不存在，则函数将返回0。
 */
size_t get_zip_file_size(const std::string &zip_file);
}  // namespace atom::io

#endif