#include "../src/io/compress.hpp"

#include <iostream>
#include <string>

using namespace OpenAPT::File;

int main()
{
    std::string file_name = "clients.json";
    std::string output_folder = "./";

    // 调用压缩单个文件函数
    bool compression_success = compress_file(file_name, output_folder);
    if (compression_success)
    {
        std::cout << "File compression successful." << std::endl;
    }
    else
    {
        std::cout << "File compression failed." << std::endl;
    }

    file_name = "clients.json.gz";
    bool decompression_success = decompress_file(file_name, output_folder);
    if (decompression_success)
    {
        std::cout << "File decompression successful." << std::endl;
    }
    else
    {
        std::cout << "File decompression failed." << std::endl;
    }

    std::string source_folder = "./log";
    std::string created_zip_file = "created.zip";

    // 创建 ZIP 文件
    bool success = create_zip(source_folder, created_zip_file);
    if (success)
    {
        std::cout << "ZIP 文件创建成功！" << std::endl;
    }
    else
    {
        std::cerr << "ZIP 文件创建失败。" << std::endl;
    }

    return 0;
}
