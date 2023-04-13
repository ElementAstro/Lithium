/*
 * crash.cpp
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

Date: 2023-4-4

Description: Crash Report

**************************************************/

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <sstream>
#include <spdlog/spdlog.h>
#include <cstring>
#include <optional>
#include <iomanip>
#include <stdexcept>
#include <random>
#if defined _WIN32 || defined CYGWIN // Windows 平台
#define OS_WINDOWS
#include <windows.h>
#include <pdh.h>
#pragma comment(lib, "pdh.lib")
#elif defined APPLE // macOS 平台
#define OS_MACOS
#include <sys/sysctl.h>
#include <mach/host_info.h>
#include <mach/mach_host.h>
#else
#define OS_LINUX
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#endif

namespace OpenAPT::CrashReport
{

    // 获取系统信息
    std::string getSystemInfo()
    {
        std::stringstream ss;
        try
        {
#ifdef _WIN32
            OSVERSIONINFO osvi;
            SYSTEM_INFO si;
            std::memset(&osvi, 0, sizeof(osvi));
            std::memset(&si, 0, sizeof(si));
            osvi.dwOSVersionInfoSize = sizeof(osvi);
            GetVersionEx(&osvi);
            GetSystemInfo(&si);
            // 组装 Windows 系统信息字符串
            ss << "Operating system version: " << osvi.dwMajorVersion << "." << osvi.dwMinorVersion << "." << osvi.dwBuildNumber << "." << osvi.dwPlatformId << std::endl;
            ss << "Processor architecture: ";
            switch (si.wProcessorArchitecture)
            {
            case PROCESSOR_ARCHITECTURE_AMD64:
                ss << "x64" << std::endl;
                break;
            case PROCESSOR_ARCHITECTURE_ARM:
                ss << "ARM" << std::endl;
                break;
            case PROCESSOR_ARCHITECTURE_IA64:
                ss << "IA-64" << std::endl;
                break;
            case PROCESSOR_ARCHITECTURE_INTEL:
                ss << "x86" << std::endl;
                break;
            default:
                ss << "Unknown" << std::endl;
                break;
            }
            ss << "Physical memory size: " << si.dwTotalPhys / 1024 / 1024 << "MB" << std::endl;
#elif defined __linux__
            char distro[256] = {0};
            FILE *fp = fopen("/etc/os-release", "r");
            if (fp != nullptr)
            {
                while (!feof(fp))
                {
                    char line[256] = {0};
                    if (fgets(line, sizeof(line), fp) == NULL) {
                    }

                    if (strncmp(line, "ID=", 3) == 0)
                    {
                        strncpy(distro, line + 3, sizeof(distro) - 1);
                        strtok(distro, "\n");
                    }
                }
                fclose(fp);
            }
            // 获取 Linux 系统信息
            struct utsname name;
            uname(&name);
            ss << "Operating system version: " << distro << " " << name.release << std::endl;
            ss << "Processor architecture: " << name.machine << std::endl;
            ss << "Physical memory size: " << sysconf(_SC_PHYS_PAGES) / 1024 / 1024 << "MB" << std::endl;
#elif defined __APPLE__
            size_t size;
            std::string os_name;
            sysctlbyname("kern.osrelease", nullptr, &size, nullptr, 0);
            char *release = new char[size];
            sysctlbyname("kern.osrelease", release, &size, nullptr, 0);
            os_name = "macOS ";
            sysctlbyname("kern.osproductversion", nullptr, &size, nullptr, 0);
            char *version = new char[size];
            sysctlbyname("kern.osproductversion", version, &size, nullptr, 0);
            os_name += version;
            delete[] release;
            delete[] version;
            // 获取 macOS 系统信息
            struct host_basic_info hostinfo;
            mach_msg_type_number_t count = HOST_BASIC_INFO_COUNT;
            if (host_info(mach_host_self(), HOST_BASIC_INFO, (host_info_t)&hostinfo, &count) == KERN_SUCCESS)
            {
                ss << "Operating system version: " << os_name << std::endl;
                ss << "Processor architecture: "
                   << "x" << sizeof(void *) * 8 << std::endl;
                ss << "Physical memory size: " << hostinfo.max_mem / 1024 / 1024 << "MB" << std::endl;
            }
#endif

            std::string cpuInfo;
            std::string ramInfo;

#ifdef _WIN32
            PDH_STATUS status;
            HQUERY query = NULL;
            HCOUNTER counter = NULL;
            PDH_FMT_COUNTERVALUE value;

            status = PdhOpenQuery(NULL, 0, &query);
            if (status != ERROR_SUCCESS)
            {
                throw std::runtime_error("Failed to open PDH query for CPU usage (error code: " + std::to_string(status) + ")");
            }

            const char *instanceName = "_Total";
            status = PdhAddCounterA(query, "\\Processor(_Total)\\% Processor Time", 0, &counter);
            if (status != ERROR_SUCCESS)
            {
                throw std::runtime_error("Failed to add CPU usage counter for instance '" + std::string(instanceName) + "' (error code: " + std::to_string(status) + ")");
            }

            status = PdhCollectQueryData(query);
            if (status != ERROR_SUCCESS)
            {
                throw std::runtime_error("Failed to collect data for PDH query (error code: " + std::to_string(status) + ")");
            }

            Sleep(1000);

            status = PdhCollectQueryData(query);
            if (status != ERROR_SUCCESS)
            {
                throw std::runtime_error("Failed to collect data for PDH query (error code: " + std::to_string(status) + ")");
            }

            status = PdhGetFormattedCounterValue(counter, PDH_FMT_DOUBLE, NULL, &value);
            if (status != ERROR_SUCCESS)
            {
                throw std::runtime_error("Failed to get formatted CPU usage value for instance '" + std::string(instanceName) + "' (error code: " + std::to_string(status) + ")");
            }

            cpuInfo = "CPU usage: " + std::to_string(value.doubleValue) + "%";
            PdhCloseQuery(query);
#elif defined __linux__
            long numProcessors = sysconf(_SC_NPROCESSORS_ONLN);
            cpuInfo = "Number of processors: " + std::to_string(numProcessors);
#elif defined __APPLE__
            mach_port_t host_port;
            mach_msg_type_number_t host_size;
            host_info_data_t host_info;

            host_port = mach_host_self();
            host_size = sizeof(host_basic_info_data_t) / sizeof(integer_t);
            host_info = (host_info_data_t)malloc(host_size);

            if (host_info(host_port, HOST_BASIC_INFO, (host_info_t)host_info, &host_size) == KERN_SUCCESS)
            {
                cpuInfo = "Number of processors: " + std::to_string(host_info->max_cpus);
            }
#endif

// 获取RAM信息
#ifdef _WIN32
            MEMORYSTATUSEX memStatus;
            memStatus.dwLength = sizeof(memStatus);
            GlobalMemoryStatusEx(&memStatus);
            ramInfo = "Memory usage: " + std::to_string((memStatus.ullTotalPhys - memStatus.ullAvailPhys) / 1024 / 1024) + "/" + std::to_string(memStatus.ullTotalPhys / 1024 / 1024) + " MB (" + std::to_string(static_cast<float>(memStatus.dwMemoryLoad)) + "%)";
#elif defined __linux__
            struct sysinfo memInfo;
            sysinfo(&memInfo);
            ramInfo = "Memory usage: " + std::to_string((memInfo.totalram - memInfo.freeram) / 1024 / 1024) + "/" + std::to_string(memInfo.totalram / 1024 / 1024) + " MB (" + std::to_string(static_cast<float>(memInfo.freeram) / memInfo.totalram * 100) + "%)";
#elif defined __APPLE__
            struct task_basic_info t_info;
            mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;
            if (task_info(mach_task_self(), TASK_BASIC_INFO, (task_info_t)&t_info, &t_info_count) == KERN_SUCCESS)
            {
                ramInfo = "Memory usage: " + std::to_string(t_info.resident_size / 1024 / 1024) + " MB";
            }
#endif

            ss << cpuInfo << std::endl;
            ss << ramInfo << std::endl;
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error occurred: " << e.what() << std::endl;
            return "";
        }
        return ss.str();
    }

    std::optional<std::string> getEnvironmentInfo()
    {
        std::stringstream ss;
        std::vector<std::string> env_vars = {
            "PATH", "TMP", "TEMP", "ProgramFiles(x86)", "ProgramFiles", "SystemRoot", "APPDATA"};

        try
        {
#if defined OS_WINDOWS
            ss << "================= Windows Environment Information =================" << std::endl;
#elif defined OS_LINUX
            ss << "================== Linux Environment Information ==================" << std::endl;
#elif defined OS_MACOS
            ss << "================= macOS Environment Information =================" << std::endl;
#endif

            for (auto &env_var : env_vars)
            {
                char *env_value;
                size_t len;

#if defined OS_WINDOWS
                if (_dupenv_s(&env_value, &len, env_var.c_str()) == 0 && env_value != nullptr)
                {
                    ss << "Windows " << env_var << "=" << env_value << std::endl;
#elif defined OS_LINUX || defined OS_MACOS
                env_value = getenv(env_var.c_str());
                if (env_value != nullptr)
                {
#if defined OS_LINUX
                    ss << "Linux ";
#elif defined OS_MACOS
                    ss << "macOS ";
#endif
                    ss << env_var << "=" << env_value << std::endl;
                }
#endif
                }

                return ss.str();
            }
            catch (const std::exception &e)
            {
                std::cerr << "Exception caught: " << e.what() << std::endl;
                return std::nullopt;
            }
        }

        void saveCrashLog(const std::string &error_msg)
        {
            try
            {
                // 定义名句列表
                std::vector<std::string> quotes = {
                    "The only way to do great work is to love what you do. - Steve Jobs",
                    "Innovation distinguishes between a leader and a follower. - Steve Jobs",
                    "To be yourself in a world that is constantly trying to make you something else is the greatest accomplishment. - Ralph Waldo Emerson",
                    "Believe you can and you're halfway there. - Theodore Roosevelt",
                    "You miss 100% of the shots you don't take. - Wayne Gretzky",
                    "Success is not final, failure is not fatal: it is the courage to continue that counts. - Winston Churchill",
                    "In three words I can sum up everything I've learned about life: it goes on. - Robert Frost",
                    "It does not matter how slowly you go as long as you do not stop. - Confucius",
                    "If you want to achieve greatness stop asking for permission. - Unknown",
                    "The only person you are destined to become is the person you decide to be. - Ralph Waldo Emerson",
                    "I have not failed. I've just found 10,000 ways that won't work. - Thomas A. Edison",
                    "A successful man is one who can lay a firm foundation with the bricks others have thrown at him. - David Brinkley",
                    "Challenges are what make life interesting and overcoming them is what makes life meaningful. - Joshua J. Marine",
                    "If you cannot do great things, do small things in a great way. - Napoleon Hill",
                    "The only limit to our realization of tomorrow will be our doubts of today. - Franklin D. Roosevelt",
                    "You must be the change you wish to see in the world. - Mahatma Gandhi",
                    "The best way to predict the future is to invent it. - Alan Kay",
                    "It always seems impossible until it's done. - Nelson Mandela",
                    "Strive not to be a success, but rather to be of value. - Albert Einstein",
                    "You are never too old to set another goal or to dream a new dream. - C.S. Lewis",
                    "Quality is not an act, it is a habit. - Aristotle",
                    "Happiness is not something ready made. It comes from your own actions. - Dalai Lama XIV",
                    "You can't build a reputation on what you are going to do. - Henry Ford",
                    "I attribute my success to this: I never gave or took any excuse. - Florence Nightingale",
                    "Believe in yourself and all that you are. Know that there is something inside you that is greater than any obstacle. - Christian D. Larson",
                    "The difference between winning and losing is most often not quitting. - Walt Disney",
                    "If you can't explain it simply, you don't understand it well enough. - Albert Einstein",
                    "Your time is limited, don't waste it living someone else's life. - Steve Jobs",
                    "Don't watch the clock; do what it does. Keep going. - Sam Levenson",
                    "Start where you are. Use what you have. Do what you can. - Arthur Ashe",
                    "We become what we think about most of the time, and that's the strangest secret. - Earl Nightingale",
                    "If you don't design your own life plan, chances are you'll fall into someone else's plan. And guess what they have planned for you? Not much. - Jim Rohn",
                    "Work hard in silence, let your success be your noise. - Frank Ocean",
                    "Believe you can and you're already halfway there. - Theodore Roosevelt",
                    "People who are crazy enough to think they can change the world, are the ones who do. - Rob Siltanen",
                    "Success is not the key to happiness. Happiness is the key to success. If you love what you are doing, you will be successful. - Albert Schweitzer",
                    "If you don't make mistakes, you aren't really trying. - Coleman Hawkins",
                    "The biggest risk is not taking any risk... In a world that's changing really quickly, the only strategy that is guaranteed to fail is not taking risks. - Mark Zuckerberg",
                    "Be the change you wish to see in the world. - Mahatma Gandhi",
                    "Don't let yesterday take up too much of today. - Will Rogers",
                    "The only source of knowledge is experience. - Albert Einstein",
                    "I have not failed. I've just found 10,000 ways that won't work. - Thomas Edison",
                    "I am not a product of my circumstances. I am a product of my decisions. - Stephen Covey",
                    "Believe in yourself! Have faith in your abilities! Without a humble but reasonable confidence in your own powers you cannot be successful or happy. - Norman Vincent Peale",
                    "Education is not the learning of facts, but the training of the mind to think. - Albert Einstein",
                    "Stay hungry, stay foolish. - Steve Jobs",
                    "You can never cross the ocean until you have the courage to lose sight of the shore. - Christopher Columbus",
                    "Success is walking from failure to failure with no loss of enthusiasm. - Winston Churchill",
                    "The best way to predict your future is to create it. - Abraham Lincoln",
                    "Believe you can and you're halfway there. - Theodore Roosevelt",
                    "The only true wisdom is in knowing you know nothing. - Socrates",
                    "You are the average of the five people you spend the most time with. - Jim Rohn",
                    "I cannot change the direction of the wind, but I can adjust my sails to always reach my destination. - Jimmy Dean",
                    "Whatever the mind of man can conceive and believe, it can achieve. - Napoleon Hill",
                    "Try not to become a man of success, but rather try to become a man of value. - Albert Einstein",
                    "Always remember that you are absolutely unique. Just like everyone else. - Margaret Mead",
                    "Everything you've ever wanted is on the other side of fear. - George Addair",
                    "Programs must be written for people to read, and only incidentally for machines to execute. - Harold Abelson",
                    "Perfection is achieved not when there is nothing more to add, but rather when there is nothing more to take away. - Antoine de Saint-Exupéry",
                    "Always code as if the person who ends up maintaining your code is a violent psychopath who knows where you live. - John F. Woods",
                    "Any fool can write code that a computer can understand. Good programmers write code that humans can understand. - Martin Fowler",
                    "There are two ways to write error-free programs; only the third one works. - Alan J. Perlis",
                    "Programming is the art of telling a computer what to do. - Donald Knuth",
                    "Walking on water and developing software from a specification are easy if both are frozen. - Edward V. Berard",
                    "One of my most productive days was throwing away 1000 lines of code. - Ken Thompson",
                    "The best way to get a project done faster is to start sooner. - Jim Highsmith",
                    "Most good programmers do programming not because they expect to get paid or get adulation by the public, but because it is fun to program. - Linus Torvalds",
                    "Debugging is like being the detective in a crime movie where you are also the murderer. - Filipe Fortes",
                    "If debugging is the process of removing software bugs, then programming must be the process of putting them in. - Edsger Dijkstra",
                    "Good code is its own best documentation. As you’re about to add a comment, ask yourself, ‘How can I improve the code so that this comment isn’t needed?’ - Steve McConnell",
                    "You’ve baked a really lovely cake, but then you’ve used dog shit for frosting. - Steve Jobs",
                    "A language that doesn't affect the way you think about programming is not worth knowing. - Alan J. Perlis",
                    "The only way to do great work is to love what you do. If you haven't found it yet, keep looking. Don't settle. - Steve Jobs",
                    "If you can't explain it simply, you don't understand it well enough. - Albert Einstein",
                    "The three virtues of a programmer: Laziness, Impatience, and Hubris. - Larry Wall",
                    "Simplicity is the soul of efficiency. - Austin Freeman",
                    "Code is like humor. When you have to explain it, it’s bad. - Cory House",
                    "It’s not at all important to get it right the first time. It’s vitally important to get it right the last time. - Andrew Hunt and David Thomas",
                    "Don't worry if it doesn't work right. If everything did, you'd be out of a job. - Mosher's Law of Software Engineering",
                    "Give someone a program, you frustrate them for a day; teach them how to program, you frustrate them for a lifetime. - David Leinweber",
                    "The difference between theory and practice is that in theory, there is no difference between theory and practice. - Richard Moore",
                    "The best thing about a boolean is even if you are wrong, you are only off by a bit. - Anonymous",
                    "I'm not a great programmer; I'm just a good programmer with great habits. - Kent Beck",
                    "Any code of your own that you haven't looked at for six or more months might as well have been written by someone else. - Eagleson's Law",
                    "Talk is cheap. Show me the code. - Linus Torvalds",
                    "The computer was born to solve problems that did not exist before. - Bill Gates",
                    "Every great developer you know got there by solving problems they were unqualified to solve until they actually did it. - Patrick McKenzie",
                    "The best code is no code at all. - Jeff Atwood",
                    "Measuring programming progress by lines of code is like measuring aircraft building progress by weight. - Bill Gates",
                    "I'm convinced that about half of what separates successful entrepreneurs from the non-successful ones is pure perseverance. - Steve Jobs",
                    "Technology is just a tool. In terms of getting the kids working together and motivating them, the teacher is the most important. - Bill Gates",
                    "Most of you are familiar with the virtues of a programmer. There are three, of course: laziness, impatience, and hubris. - Larry Wall",
                    "Software and cathedrals are much the same – first we build them, then we pray. - Sam Redwine",
                    "How you look at it is pretty much how you'll see it - Rasheed Ogunlaru",
                    "If the code and the comments disagree, then both are probably wrong. - Norm Schryer",
                    "It's hard enough to find an error in your code when you're looking for it; it's even harder when you've assumed your code is error-free. - Steve McConnell",
                    "Controlling complexity is the essence of computer programming. - Brian Kernighan",
                    "Java is to JavaScript what car is to Carpet. - Chris Heilmann",
                    "A good programmer is someone who always looks both ways before crossing a one-way street. - Doug Linder",
                    "A language that doesn't have everything is actually easier to program in than some that do. - Dennis M. Ritchie",
                    "I choose a lazy person to do a hard job. Because a lazy person will find an easy way to do it. - Bill Gates",
                    "The function of good software is to make the complex appear to be simple. - Grady Booch",
                    "Sometimes it pays to stay in bed on Monday, rather than spending the rest of the week debugging Monday's code. - Dan Salomon",
                    "First, solve the problem. Then, write the code. - John Johnson",
                    "Weeks of coding can save you hours of planning. - Anonymous",
                    "Without requirements or design, programming is the art of adding bugs to an empty text file. - Louis Srygley",
                    "Hardware eventually fails. Software eventually works. - Michael Hartung"};

                std::time_t now = std::time(nullptr);
                char time_str[20];
                std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
                std::string system_info = getSystemInfo();
                auto environment_info = getEnvironmentInfo();

                // 组装日志信息字符串
                std::stringstream ss;
                ss << "Program crashed at: " << time_str << std::endl;
                ss << "Error message: " << error_msg << std::endl;
                ss << "==================== System Information ====================" << std::endl;
                ss << system_info << std::endl;
                ss << "================= Environment Variables Information ==================" << std::endl;
                if (environment_info.has_value())
                {
                    ss << environment_info.value() << std::endl;
                }
                else
                {
                    ss << "Failed to get environment information." << std::endl;
                }

                // 随机选择名句
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> dis(0, quotes.size() - 1);
                ss << "============ Famous saying: " << quotes[dis(gen)] << " ============" << std::endl;

                // 写入日志文件
                std::stringstream sss;
                sss << "crash_report/crash_" << std::put_time(std::localtime(&now), "%Y%m%d_%H%M%S") << ".log";

                // 检查目录是否存在，如果不存在则创建
                std::filesystem::path dir_path("crash_report");
                if (!std::filesystem::exists(dir_path)) {
                    std::filesystem::create_directory(dir_path);
                }

                // 组装日志信息字符串

                // 写入日志文件
                std::ofstream ofs(sss.str());
                if (ofs.good())
                {
                    ofs << ss.str();
                    ofs.close();
                }
                else
                {
                    throw std::runtime_error("Failed to open log file.");
                }
            }
            catch (const std::exception &e)
            {
                std::cerr << "Exception caught: " << e.what() << std::endl;
            }
        }
    }
