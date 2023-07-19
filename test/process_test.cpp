#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <thread>
#include <chrono>
#include <mutex>
#include <fstream>
#include <loguru.hpp>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#endif

using namespace std;

struct Process
{
    pid_t pid;
    std::string name;
    std::string output;
};

class ProcessManager
{
private:
    std::vector<Process> processes;
    std::mutex mtx;

public:
    void createProcess(const std::string &command, const std::string &identifier)
    {
        pid_t pid;

#ifdef _WIN32
        STARTUPINFO si{};
        PROCESS_INFORMATION pi{};
        std::string cmd = "powershell.exe -Command \"" + command + "\"";
        if (!CreateProcess(NULL, (LPSTR)cmd.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
        {
            LOG_F(ERROR, "Failed to create PowerShell process");
            return;
        }
        pid = pi.dwProcessId;
#else
        pid = fork();

        if (pid == 0)
        {
            // Child process code
            LOG_F(INFO, "Running command: %s", command.c_str());
            int pipefd[2];
            pipe(pipefd);
            dup2(pipefd[1], STDOUT_FILENO);
            close(pipefd[0]);
            close(pipefd[1]);

            execlp(command.c_str(), command.c_str(), NULL);
            exit(0);
        }
        else if (pid < 0)
        {
            // Error handling
            PLOG_F(ERROR, "Failed to create process");
            return;
        }
#endif

        std::unique_lock<std::mutex> lock(mtx);
        Process process;
        process.pid = pid;
        process.name = identifier;
        processes.push_back(process);
        lock.unlock();
        LOG_F(INFO, "Process created: %s (PID: %d)", identifier.c_str(), pid);
    }

    void runScript(const std::string &script, const std::string &identifier)
    {
        pid_t pid;

#ifdef _WIN32
        std::string cmd = "powershell.exe -Command \"" + script + "\"";
        STARTUPINFO si{};
        PROCESS_INFORMATION pi{};
        if (!CreateProcess(NULL, (LPSTR)cmd.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
        {
            LOG_F(ERROR, "Failed to create process");
            return;
        }
        pid = pi.dwProcessId;
#else
        pid = fork();

        if (pid == 0)
        {
            // Child process code
            LOG_F(INFO, "Running script: %s", script.c_str());

#ifdef __APPLE__
            execl("/bin/sh", "sh", "-c", script.c_str(), NULL);
#else
            execl("/bin/bash", "bash", "-c", script.c_str(), NULL);
#endif

            exit(0);
        }
        else if (pid < 0)
        {
            // Error handling
            LOG_F(ERROR, "Failed to create process");
            return;
        }
#endif

        std::unique_lock<std::mutex> lock(mtx);
        Process process;
        process.pid = pid;
        process.name = identifier;
        processes.push_back(process);
        lock.unlock();
        LOG_F(INFO, "Process created: %s (PID: %d)", identifier.c_str(), pid);
    }

    void terminateProcess(pid_t pid, int signal = SIGTERM)
    {
        auto it = std::find_if(processes.begin(), processes.end(), [pid](const Process &p)
                               { return p.pid == pid; });

        if (it != processes.end())
        {
#ifdef _WIN32
            HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
            if (hProcess != NULL)
            {
                TerminateProcess(hProcess, 0);
                CloseHandle(hProcess);
                LOG_F(INFO, "Process terminated: %s (PID: %d)", it->name.c_str(), pid);
            }
            else
            {
                LOG_F(ERROR, "Failed to terminate process");
            }
#else
            int status;
            kill(pid, signal); // Send signal to the process
            waitpid(pid, &status, 0);

            std::unique_lock<std::mutex> lock(mtx);
            LOG_F(INFO, "Process terminated: %s (PID: %d)", it->name.c_str(), pid);
            lock.unlock();
#endif

            processes.erase(it);
        }
        else
        {
            LOG_F(ERROR, "Process not found");
        }
    }

    void listProcesses()
    {
        std::unique_lock<std::mutex> lock(mtx);
        LOG_F(INFO, "Currently running processes:");

        for (const auto &process : processes)
        {
            LOG_F(INFO, "%s (PID: %d)", process.name.c_str(), process.pid);
        }

        lock.unlock();
    }

    std::vector<std::string> getProcessOutput(const std::string &identifier)
    {
        auto it = std::find_if(processes.begin(), processes.end(), [&identifier](const Process &p)
                               { return p.name == identifier; });

        if (it != processes.end())
        {
            std::vector<std::string> outputLines;
            std::stringstream ss(it->output);
            std::string line;

            while (getline(ss, line))
            {
                outputLines.push_back(line);
            }

            return outputLines;
        }
        else
        {
            LOG_F(ERROR, "Process not found");
            return std::vector<std::string>();
        }
    }

    void waitForCompletion()
    {
        for (const auto &process : processes)
        {
#ifdef _WIN32
            HANDLE hProcess = OpenProcess(SYNCHRONIZE, FALSE, process.pid);
            if (hProcess != NULL)
            {
                WaitForSingleObject(hProcess, INFINITE);
                CloseHandle(hProcess);
                LOG_F(INFO, "Process completed: %s (PID: %d)", process.name.c_str(), process.pid);
            }
            else
            {
                LOG_F(ERROR, "Failed to wait for process completion");
            }
#else
            int status;
            waitpid(process.pid, &status, 0);

            std::unique_lock<std::mutex> lock(mtx);
            LOG_F(INFO, "Process completed: %s (PID: %d)", process.name.c_str(), process.pid);
            lock.unlock();
#endif
        }

        processes.clear();
    }
};

void testProcessManager()
{
    ProcessManager manager;

    manager.createProcess("echo \"Hello from PowerShell\"", "ps1");

    vector<string> output = manager.getProcessOutput("ps1");

    if (!output.empty())
    {
        cout << "Output of 'ls' command:" << endl;

        for (const auto &line : output)
        {
            cout << line << endl;
        }
    }

    manager.runScript("./test.ps1", "ps11");

    // List current processes
    manager.listProcesses();

    // Wait for all processes to complete
    manager.waitForCompletion();

    // List processes after completion
    manager.listProcesses();

    // Get process outpu
}

int main()
{
    loguru::add_file("process.log", loguru::Truncate, loguru::Verbosity_INFO);

    testProcessManager();

    return 0;
}
