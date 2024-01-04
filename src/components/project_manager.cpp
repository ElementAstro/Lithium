#include "project_manager.hpp"

#include <cstdio>
#include <ctime>

#include "atom/log/loguru.hpp"

namespace Lithium
{
    bool ProjectManager::addProject(const std::shared_ptr<Project> &project)
    {
        m_projects.push_back(project);
        DLOG_F(INFO, "Added project: {}", project->getName());
    }

    bool ProjectManager::removeProject(const std::string &name)
    {
        for (auto it = m_projects.begin(); it != m_projects.end(); ++it)
        {
            if (it->getName() == name)
            {
                m_projects.erase(it);
                DLOG_F(INFO, "Removed project: {}", name);
                return;
            }
        }
        DLOG_F(ERROR, "Failed to remove project: {}", name);
    }

    void ProjectManager::listProjects() const
    {
        std::cout << "项目列表：" << std::endl;
        for (const auto &project : m_projects)
        {
            std::cout << "- " << project.getName() << std::endl;
        }
    }

    std::pair<bool, std::string> ProjectManager::buildProject(const std::string &name)
    {
        for (auto &project : m_projects)
        {
            if (project.getName() == name)
            {
                std::cout << "开始构建项目：" << project.getName() << std::endl;

                // 记录构建开始时间
                std::time_t startTime = std::time(nullptr);

                // 使用 popen 执行构建指令，并获取输出
                FILE *pipe = popen(project.getBuildCommand().c_str(), "r");
                if (!pipe)
                {
                    return {false, "无法执行构建命令"};
                }

                char buffer[128];
                std::string output;
                while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
                {
                    output += buffer;
                }

                int result = pclose(pipe);

                // 记录构建结束时间
                std::time_t endTime = std::time(nullptr);

                if (result == 0)
                {
                    std::cout << "项目构建成功！" << std::endl;
                    project.setLastBuildStatus("成功");
                }
                else
                {
                    std::cout << "项目构建失败！" << std::endl;
                    project.setLastBuildStatus("失败");
                }

                // 更新最后一次构建时间
                project.setLastBuildTime(endTime);

                return {result == 0, output};
            }
        }

        std::cout << "未找到项目：" << name << std::endl;
        return {false, ""};
    }

    void ProjectManager::showProjectDetails(const std::string &name) const
    {
        for (const auto &project : m_projects)
        {
            if (project.getName() == name)
            {
                std::cout << "项目名称：" << project.getName() << std::endl;
                std::cout << "构建指令：" << project.getBuildCommand() << std::endl;
                std::cout << "最后一次构建状态：" << project.getLastBuildStatus() << std::endl;

                if (project.getLastBuildTime() != 0)
                {
                    std::tm *timeInfo = std::localtime(&project.getLastBuildTime());
                    char buffer[80];
                    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeInfo);
                    std::cout << "最后一次构建时间：" << buffer << std::endl;
                }

                return;
            }
        }

        std::cout << "未找到项目：" << name << std::endl;
    }

} // namespace Lithium
