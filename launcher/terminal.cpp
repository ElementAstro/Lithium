/*
 * terminal.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-30

Description: Terminal

**************************************************/

#include "terminal.hpp"

namespace Lithium::Terminal
{
    CommandManager::CommandManager() : hist_iter_(command_history_.begin()) {}

    void CommandManager::registerCommand(const std::string &cmd, std::function<std::string(const std::string &)> func)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        commands_.emplace(cmd, func);
    }

    const std::string &CommandManager::runCommand(const std::string &cmd, const std::string &arg)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = commands_.find(cmd);
        if (it != commands_.end())
        {
            const std::string command_str = cmd + " " + arg;
            command_history_.emplace_back(command_str);
            history_index_ = command_history_.size();
            static std::string result = it->second(arg);
            return result;
        }
        static const std::string unknown_cmd = "\033[31mUnknown command: " + cmd + "\033[0m\n";
        return unknown_cmd;
    }

    std::future<std::string> CommandManager::runCommandAsync(const std::string &cmd, const std::string &arg)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = commands_.find(cmd);
        if (it != commands_.end())
        {
            std::future<std::string> future = std::async(std::launch::async, [this, it, arg]()
                                                         {
            const std::string command_str = it->first + " " + arg;
            command_history_.emplace_back(command_str);
            history_index_ = command_history_.size();
            return std::string(it->second(arg)); });
            addFuture(std::move(future));
            return future;
        }
        return std::async(std::launch::async, []()
                          { return std::string("\033[31mUnknown command\033[0m"); });
    }

    void CommandManager::addFuture(std::future<std::string> &&future)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        futures_.emplace_back(std::move(future));
    }

    void CommandManager::join()
    {
        for (auto it = futures_.begin(); it != futures_.end();)
        {
            if (it->valid())
            {
                try
                {
                    it->wait();
                    const std::string result = it->get();
                    if (!result.empty())
                    {
                        std::cout << result << std::endl;
                    }
                    it = futures_.erase(it);
                }
                catch (const std::exception &e)
                {
                    std::cerr << "Exception in task: " << e.what() << std::endl;
                    it = futures_.erase(it);
                }
            }
            else
            {
                it = futures_.erase(it);
            }
        }
    }

    const std::vector<std::string> &CommandManager::getRegisteredCommands() const
    {
        std::vector<std::string> commands;
        for (const auto &cmd : commands_)
        {
            commands.emplace_back(cmd.first);
        }
        static const std::vector<std::string> command_list = std::move(commands);
        return command_list;
    }

    const std::string &CommandManager::getPrevCommand()
    {
        if (hist_iter_ != command_history_.begin())
        {
            --hist_iter_;
            history_index_ = hist_iter_ - command_history_.begin();
        }
        static const std::string empty_str;
        return (history_index_ >= 0 && history_index_ < command_history_.size()) ? *(hist_iter_) : empty_str;
    }

    const std::string &CommandManager::getNextCommand()
    {
        if (hist_iter_ != command_history_.end() - 1)
        {
            ++hist_iter_;
            history_index_ = (hist_iter_ - command_history_.begin());
        }
        static const std::string empty_str;
        return (history_index_ >= 0 && history_index_ < command_history_.size()) ? *(hist_iter_) : empty_str;
    }

    void CommandManager::addCommandHistory(const std::string &cmd)
    {
        command_history_.emplace_back(cmd);
        history_index_ = command_history_.size();
        hist_iter_ = command_history_.end();
    }

    bool CommandManager::hasNextCommand() const
    {
        return hist_iter_ != command_history_.end() - 1;
    }

    bool CommandManager::hasPrevCommand() const
    {
        return hist_iter_ != command_history_.begin();
    }

    std::string getCursorLocation()
    {
        std::cout << "\033[6n" << std::flush;

        char c = '\0';
        std::string result = "";
        while (c != 'R')
        {
            if (read(STDIN_FILENO, &c, 1) == -1)
            {
            }
            result += c;
        }

        return result;
    }

    bool isColorSupported()
    {
#ifdef _WIN32
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hOut == INVALID_HANDLE_VALUE)
            return false;

        DWORD dwMode;
        if (!GetConsoleMode(hOut, &dwMode))
            return false;

        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        if (!SetConsoleMode(hOut, dwMode))
            return false;

        // 输出绿色字符
        std::cout << "\x1B[32m" << std::flush;

        int c = getchar();           // 读取用户的输入
        bool result = (c == '\033'); // 判断是否为控制字符

        // 恢复默认颜色
        std::cout << "\x1B[0m" << std::flush;

        // 还原控制台模式
        dwMode &= ~ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hOut, dwMode);

        return result;
#else
        if (!isatty(fileno(stdout)))
            return false;

        struct termios saved_termios, modified_termios;
        tcgetattr(STDIN_FILENO, &saved_termios);

        modified_termios = saved_termios;
        modified_termios.c_lflag &= ~(ECHO | ICANON);
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &modified_termios);

        // 输出绿色字符
        std::cout << "\033[32m" << std::flush;

        int c = getchar();           // 读取用户的输入
        bool result = (c == '\033'); // 判断是否为控制字符

        // 恢复默认颜色
        std::cout << "\033[0m" << std::flush;

        tcsetattr(STDIN_FILENO, TCSAFLUSH, &saved_termios);

        return result;
#endif
    }

    std::string getTerminalInput(CommandManager &manager)
    {
        // 保存当前终端属性
#ifdef _WIN32
        HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
        DWORD mode;
        GetConsoleMode(hStdin, &mode);
        DWORD saved_mode = mode;
#else
        struct termios saved_termios, modified_termios;
        tcgetattr(STDIN_FILENO, &saved_termios);
#endif

        // 修改终端属性
#ifdef _WIN32
        mode &= ~ENABLE_ECHO_INPUT;
        mode &= ~ENABLE_LINE_INPUT;
        SetConsoleMode(hStdin, mode);
#else
        modified_termios = saved_termios;
        modified_termios.c_lflag &= ~(ECHO | ICANON);
        modified_termios.c_cc[VTIME] = 0; // 设置非阻塞模式
        modified_termios.c_cc[VMIN] = 1;  // 设置最少读取的字符数
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &modified_termios);
#endif

        std::string input = "";
        int c = 0;

        std::cout << "\033[94m>>>\033[0m " << std::flush; // 改变提示符颜色

        while (true)
        {
            // 读取字符
#ifdef _WIN32
            c = _getch();
#else
            c = getchar();
#endif

            if (c == '\n' || c == EOF)
            {                           // 输入结束，退出循环
                std::cout << std::endl; // 输出一个换行符

                if (!input.empty())
                {
                    // 将输入的命令添加到历史记录中
                    manager.addCommandHistory(input);

                    std::string output = manager.runCommand(input, "");
                    std::cout << output;
                }

                std::cout << "\033[94m>>>\033[0m " << std::flush; // 重新输出提示符

                input.clear(); // 清空输入缓存
            }
            else if (c == 127 || c == 8)
            { // 处理退格键
                if (!input.empty())
                {
                    input.pop_back();

#ifdef _WIN32
                    std::cout << "\b \b" << std::flush; // 输出退格符、空格和退格符，相当于清空当前字符
#else
                    std::cout << "\b \b" << std::flush; // 输出退格符、空格和退格符，相当于清空当前字符
#endif
                }
            }
            else if (c == '\033')
            { // 处理特殊字符，如向上键
#ifdef _WIN32
                int nextC = _getch();
#else
                int nextC = getchar();
#endif
                if (nextC == '[')
                {
#ifdef _WIN32
                    switch (_getch())
#else
                    switch (getchar())
#endif
                    {
                    case 'B': // 向上键
                        if (manager.hasNextCommand())
                        {
                            input = manager.getNextCommand();
                            std::cout << "\r\033[K"
                                      << "\033[94m>>>\033[0m " << input << std::flush; // 输出命令和提示符
                        }
                        break;

                    case 'A': // 向下键
                        if (manager.hasPrevCommand())
                        {
                            input = manager.getPrevCommand();
                            std::cout << "\r\033[K"
                                      << "\033[94m>>>\033[0m " << input << std::flush; // 输出命令和提示符
                        }
                        break;

                    default:
                        break;
                    }
                }
            }
            else
            { // 处理普通字符
                input += c;
                std::cout << static_cast<char>(c) << std::flush; // 直接输出字符即可
            }
        }

        // 恢复终端属性
#ifdef _WIN32
        SetConsoleMode(hStdin, saved_mode);
#else
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &saved_termios);
#endif

        return input;
    }

    // 实现 ls 指令，用于显示当前目录下的文件和文件夹
    std::string lsCommand(const std::string &arg)
    {
        std::string cmd = "ls -al ";
        if (!arg.empty())
        {
            cmd += arg;
        }
        else
        {
            cmd += ".";
        }

        FILE *pipe = popen(cmd.c_str(), "r");
        if (!pipe)
        {
            return "Failed to execute command: " + cmd;
        }

        char buffer[128];
        std::string result = "";
        while (fgets(buffer, 128, pipe) != nullptr)
        {
            result += buffer;
        }
        pclose(pipe);

        return result;
    }

    // 实现 pwd 指令，用于显示当前工作目录路径名
    std::string pwdCommand(const std::string &arg)
    {
        return "Current working directory: " + std::string(getcwd(nullptr, 0)) + "\n";
    }

    void printHeader()
    {
        std::cout << "Welcome to Lithium Command Line Tool v1.0" << std::endl;
        std::cout << "Type 'help' to see a list of available commands." << std::endl;
        std::cout << "--------------------------------------------------" << std::endl;
    }

    // 实现 mkdir 指令，用于创建目录
    std::string mkdirCommand(const std::string &arg)
    {
        std::string cmd = "mkdir " + arg;

        FILE *pipe = popen(cmd.c_str(), "r");
        if (!pipe)
        {
            return "Failed to execute command: " + cmd;
        }

        pclose(pipe);

        return "Directory created: " + arg;
    }

    // 实现 cp 指令，用于复制文件或目录
    std::string cpCommand(const std::string &arg)
    {
        std::string cmd = "cp -r " + arg;

        FILE *pipe = popen(cmd.c_str(), "r");
        if (!pipe)
        {
            return "Failed to execute command: " + cmd;
        }

        pclose(pipe);

        return "File or directory copied: " + arg;
    }

    // 实现 help 指令，列出所有可用的命令
    std::string helpCommand(CommandManager &manager, const std::string &arg)
    {
        std::vector<std::string> commands = manager.getRegisteredCommands();

        // 构造输出字符串
        std::stringstream ss;
        ss << "Available commands:" << std::endl;
        for (const auto &cmd : commands)
        {
            ss << " - " << cmd << std::endl;
        }
        return ss.str();
    }

    std::string systemCommand(const std::string &arg)
    {
        std::string command = "sh -c '" + arg + "' 2>&1";
        std::string output;
        int result = std::system(command.c_str());
        if (result == 0)
        {
            output = "\033[32mCommand executed successfully.\033[0m\n";
        }
        else
        {
            output = "\033[31mCommand failed to execute.\033[0m\n";
        }
        return output;
    }
}

using namespace Lithium::Terminal;

int main()
{
    CommandManager manager;

    // 注册指令函数
    manager.registerCommand("ls", lsCommand);
    manager.registerCommand("pwd", pwdCommand);
    manager.registerCommand("mkdir", mkdirCommand);
    manager.registerCommand("cp", cpCommand);
    manager.registerCommand("system", systemCommand);

    // 打印终端头部信息
    printHeader();

    while (true)
    {
        // 获取终端输入
        std::string input = getTerminalInput(manager);

        // 运行指令函数
        std::string result = manager.runCommand(input, "");

        // 在终端上显示执行结果
        std::cout << result << std::endl;
    }

    return 0;
}
