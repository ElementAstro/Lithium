/*
 * daemon.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
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

Date: 2023-11-11

Description: Daemon thread implementation

**************************************************/

#include "daemon.hpp"

#include "atom/log/loguru.hpp"

std::string ProcessInfo::toString() const
{
    std::stringstream ss;
    ss << "[ProcessInfo parent_id=" << ProcessInfoMgr::GetInstance()->parent_id
       << " main_id=" << ProcessInfoMgr::GetInstance()->main_id
       << " parent_start_time=" << Time2Str(ProcessInfoMgr::GetInstance()->parent_start_time)
       << " main_start_time=" << Time2Str(ProcessInfoMgr::GetInstance()->main_start_time)
       << " restart_count=" << ProcessInfoMgr::GetInstance()->restart_count << "]";
    return ss.str();
}

int ProcessInfo::real_start(int argc, char **argv,
                            std::function<int(int argc, char **argv)> main_cb)
{
    ProcessInfoMgr::GetInstance()->main_id = getpid();
    ProcessInfoMgr::GetInstance()->main_start_time = time(0);
    return main_cb(argc, argv);
}

int ProcessInfo::real_daemon(int argc, char **argv,
                             std::function<int(int argc, char **argv)> main_cb)
{
#ifdef _WIN32
    // Create new process to hide window and console output
    STARTUPINFO si = {sizeof(si)};
    PROCESS_INFORMATION pi;
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    if (!CreateProcess(NULL, GetCommandLine(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
    {
        LOG_F(ERROR, "Create daemon process failed");
        return -1;
    }

    // Exit parent process
    ExitProcess(0);
#else
    daemon(1, 0);
    ProcessInfoMgr::GetInstance()->parent_id = getpid();
    ProcessInfoMgr::GetInstance()->parent_start_time = time(0);
    while (true)
    {
        pid_t pid = fork();
        if (pid == 0)
        {
            // Child process returns
            ProcessInfoMgr::GetInstance()->main_id = getpid();
            ProcessInfoMgr::GetInstance()->main_start_time = time(0);
            LOG_F(INFO, "daemon process start pid={} argv={}", getpid(), argv)
            return real_start(argc, argv, main_cb);
        }
        else if (pid < 0)
        {
            LOG_F(ERROR, "fork fail return={} errno={} errstr={}", pid, errno, strerror(errno));
            return -1;
        }
        else
        {
            // Parent process
            // Parent process returns
            int status = 0;
            waitpid(pid, &status, 0);

            // Terminated abnormally
            if (status)
            {
                if (status == 9)
                {
                    LOG_F(INFO, "daemon process killed pid={}", getpid());
                    break;
                }
                else
                {
                    LOG_F(ERROR, "child crash pid={} status={}", pid, status);
                }
            }
            else
            {
                LOG_F(INFO, "daemon process restart pid={}", getpid());
                break;
            }

            // Restart child process
            ProcessInfoMgr::GetInstance()->restart_count += 1;
            sleep(g_daemon_restart_interval->getValue());
        }
    }
#endif

    return 0;
}

int ProcessInfo::start_daemon(int argc, char **argv,
                              std::function<int(int argc, char **argv)> main_cb,
                              bool is_daemon)
{
#ifdef _WIN32
    if (!is_daemon)
    {
        ProcessInfoMgr::GetInstance()->parent_id = getpid();
        ProcessInfoMgr::GetInstance()->parent_start_time = time(0);
        return real_start(argc, argv, main_cb);
    }
#else
    if (!is_daemon)
    {
        ProcessInfoMgr::GetInstance()->parent_id = getpid();
        ProcessInfoMgr::GetInstance()->parent_start_time = time(0);
        return real_start(argc, argv, main_cb);
    }
#endif

    return real_daemon(argc, argv, main_cb);
}

int main_cb(int argc, char **argv)
{
    std::cout << "Hello from main_cb!" << std::endl;
    return 0;
}

int main(int argc, char **argv)
{
    ProcessInfo processInfo;
    processInfo.start_daemon(argc, argv, std::bind(main_cb, argc, argv), false);
    return 0;
}