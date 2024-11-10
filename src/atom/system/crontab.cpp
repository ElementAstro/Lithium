#include "crontab.hpp"

#include <fstream>

#include "atom/type/json.hpp"

using json = nlohmann::json;

auto CronJob::toJson() const -> json {
    return json{{"time", time_}, {"command", command_}};
}

auto CronJob::fromJson(const json& jsonObj) -> CronJob {
    return CronJob{jsonObj.at("time").get<std::string>(),
                   jsonObj.at("command").get<std::string>()};
}

auto CronManager::createCronJob(const CronJob& job) -> bool {
    std::string command = "crontab -l 2>/dev/null | { cat; echo \"" +
                          job.time_ + " " + job.command_ + "\"; } | crontab -";
    if (system(command.c_str()) == 0) {
        jobs_.push_back(job);
        return true;
    }
    return false;
}

auto CronManager::deleteCronJob(const std::string& command) -> bool {
    std::string jobToDelete = " " + command;
    std::string cmd =
        "crontab -l | grep -v \"" + jobToDelete + "\" | crontab -";
    if (system(cmd.c_str()) == 0) {
        jobs_.erase(std::remove_if(jobs_.begin(), jobs_.end(),
                                   [&](const CronJob& job) {
                                       return job.command_ == command;
                                   }),
                    jobs_.end());
        return true;
    }
    return false;
}

auto CronManager::listCronJobs() -> std::vector<CronJob> {
    std::vector<CronJob> currentJobs;
    std::string cmd = "crontab -l";
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe)
        return currentJobs;

    constexpr size_t bufferSize = 128;
    char buffer[bufferSize];
    while (fgets(buffer, sizeof buffer, pipe) != nullptr) {
        std::string line(buffer);
        size_t spacePos = line.find(' ');
        if (spacePos != std::string::npos) {
            currentJobs.push_back(
                {line.substr(0, spacePos), line.substr(spacePos + 1)});
        }
    }
    pclose(pipe);
    return currentJobs;
}

auto CronManager::exportToJSON(const std::string& filename) -> bool {
    json jsonObj;
    for (const auto& job : jobs_) {
        jsonObj.push_back(job.toJson());
    }
    std::ofstream file(filename);
    if (file.is_open()) {
        file << jsonObj.dump(4);
        return true;
    }
    return false;
}

auto CronManager::importFromJSON(const std::string& filename) -> bool {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    json jsonObj;
    file >> jsonObj;

    for (const auto& jobJson : jsonObj) {
        CronJob job = CronJob::fromJson(jobJson);
        createCronJob(job);
    }
    return true;
}

auto CronManager::updateCronJob(const std::string& oldCommand,
                                const CronJob& newJob) -> bool {
    if (deleteCronJob(oldCommand)) {
        return createCronJob(newJob);
    }
    return false;
}

auto CronManager::viewCronJob(const std::string& command) -> CronJob {
    auto iterator = std::find_if(
        jobs_.begin(), jobs_.end(),
        [&](const CronJob& job) { return job.command_ == command; });
    return (iterator != jobs_.end()) ? *iterator
                                     : CronJob{"", ""};  // 返回一个空的任务
}

auto CronManager::searchCronJobs(const std::string& query)
    -> std::vector<CronJob> {
    std::vector<CronJob> foundJobs;
    for (const auto& job : jobs_) {
        if (job.command_.find(query) != std::string::npos ||
            job.time_.find(query) != std::string::npos) {
            foundJobs.push_back(job);
        }
    }
    return foundJobs;
}

auto CronManager::statistics() -> int { return static_cast<int>(jobs_.size()); }