/*
 * compress.hpp
 * 
 * Copyright (C) 2023 Max Qian <lightapt.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/************************************************* 
 
Copyright: 2023 Max Qian. All rights reserved
 
Author: Max Qian

E-mail: astro_air@126.com
 
Date: 2023-3-31
 
Description: Compressor using ZLib
 
**************************************************/

#ifndef _COMPRESS_HPP_
#define _COMPRESS_HPP_

namespace OpenAPT::Compressor {

    /**
     * 对单个文件进行压缩
     * @param file_name 待压缩的文件名（包含路径）
     * @return 是否压缩成功
     */
    bool compress_file(const char *file_name);

    /**
     * 对单个文件进行解压缩
     * @param file_name 待解压的文件名（包含路径）
     * @return 是否解压成功
     */
    bool decompress_file(const char *file_name);

    /**
     * 对指定目录下的文件进行压缩
     * @param folder_name 待压缩的目录名（绝对路径）
     * @return 是否压缩成功
     */
    bool compress_folder(const char *folder_name);
}

#endif