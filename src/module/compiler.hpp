#pragma once

#include <string>
#include <unordered_map>

class Compiler
{
public:
    /**
     * \brief 编译 C++ 代码为共享库，并加载到内存中
     * 
     * \param code 要编译的代码
     * \param moduleName 模块名
     * \param functionName 入口函数名
     * \return 编译是否成功
     */
    bool CompileToSharedLibrary(const std::string& code, const std::string& moduleName, const std::string& functionName);

private:
    /**
     * \brief 复制文件
     * 
     * \param source 源文件路径
     * \param destination 目标文件路径
     * \return 是否复制成功
     */
    bool CopyFile(const std::string& source, const std::string& destination);

    /**
     * \brief 运行外部 shell 命令，并将标准输入输出流转发到命令的标准输入输出流中
     * 
     * \param command 要运行的命令
     * \param inputStream 标准输入流
     * \param outputStream 标准输出流
     * \return 命令运行的返回值
     */
    int RunShellCommand(const std::string& command, std::istream& inputStream, std::ostream& outputStream);

    std::unordered_map<std::string, std::string> cache_;
};

