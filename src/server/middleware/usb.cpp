#include "usb.hpp"

#include <filesystem>

#include "config/configor.hpp"

#include "atom/async/message_bus.hpp"
#include "atom/function/global_ptr.hpp"
#include "atom/io/file_permission.hpp"
#include "atom/log/loguru.hpp"
#include "atom/sysinfo/disk.hpp"
#include "atom/system/command.hpp"
#include "atom/system/env.hpp"
#include "atom/type/json.hpp"
#include "atom/utils/print.hpp"
#include "utils/constant.hpp"

namespace lithium::middleware {
namespace internal {
auto parseString(const std::string& input,
                 const std::string& imgFilePath) -> std::vector<std::string> {
    std::vector<std::string> paths;
    std::string baseString;

    // 查找第一个'{'
    size_t pos = input.find('{');
    if (pos != std::string::npos) {
        // 获取 baseString
        baseString = input.substr(0, pos);

        // 获取 '{' 后的内容
        std::string content = input.substr(pos + 1);

        // 查找配对的 '}'
        size_t endPos = content.find('}');
        if (endPos != std::string::npos) {
            content = content.substr(0, endPos);

            // 去掉末尾的分号（如果有的话）
            if (!content.empty() && content.back() == ';') {
                content.pop_back();
            }

            // 分割 content
            size_t start = 0;
            size_t end;
            while ((end = content.find(';', start)) != std::string::npos) {
                std::string part = content.substr(start, end - start);
                // 去掉可能的空部分
                if (!part.empty()) {
                    // 拼接完整的路径并添加到路径列表
                    paths.push_back(std::filesystem::path(imgFilePath) /
                                    baseString / part);
                }
                start = end + 1;
            }
            // 添加最后一个部分（如果存在）
            if (start < content.size()) {
                std::string part = content.substr(start);
                if (!part.empty()) {
                    paths.push_back(std::filesystem::path(imgFilePath) /
                                    baseString / part);
                }
            }
        }
    }
    return paths;
}

bool remountReadWrite(const std::string& mountPoint,
                      const std::string& password) {
    std::ostringstream commandStream;
    commandStream << "echo '" << password << "' | sudo -S mount -o remount,rw "
                  << mountPoint;
    std::string command = commandStream.str();
    return system(command.c_str()) == 0;
}

long long getUSBSpace(const std::string& path) {
    try {
        auto spaceInfo = fs::space(path);
        return spaceInfo.available;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
        return -1;
    }
}

long long getTotalSize(const std::vector<std::string>& paths) {
    long long totalSize = 0;
    for (const auto& path : paths) {
        try {
            totalSize += fs::file_size(path);
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Filesystem error: " << e.what() << std::endl;
        }
    }
    return totalSize;
}
}  // namespace internal

void moveImageToUSB(const std::string& path) {
    LOG_F(INFO, "moveImageToUSB: Entering function with path: {}",
          path.c_str());

    std::shared_ptr<ConfigManager> configManager;
    GET_OR_CREATE_PTR(configManager, ConfigManager, Constants::CONFIG_MANAGER)
    std::string imageBasePath =
        configManager->getValue("/lithium/image/base_path")
            .value_or("~/images");
    std::vector<std::string> files = internal::parseString(path, imageBasePath);

    std::shared_ptr<atom::utils::Env> env;
    GET_OR_CREATE_PTR(env, atom::utils::Env, Constants::ENVIRONMENT)
    std::string basePath = "/media/" + env->getEnv("USER");
    if (!fs::exists(basePath)) {
        LOG_F(ERROR, "moveImageToUSB: Base directory does not exist.");
        return;
    }

    std::vector<std::string> folderList;
    for (const auto& entry : fs::directory_iterator(basePath)) {
        if (entry.is_directory() && entry.path().filename() != "CDROM") {
            folderList.push_back(entry.path().string());
        }
    }

    if (folderList.size() != 1) {
        std::string errorMsg = folderList.empty()
                                   ? "ImageSaveError:USB-Null"
                                   : "ImageSaveError:USB-Multiple";
        LOG_F(ERROR, "moveImageToUSB: {}", errorMsg.c_str());
        return;
    }

    std::string usbMountPoint = folderList.front();
    LOG_F(INFO, "moveImageToUSB: USB mount point: {}", usbMountPoint.c_str());

    const std::string PASSWORD = "quarcs";  // sudo 密码

    if (!fs::exists(usbMountPoint) || !fs::is_directory(usbMountPoint)) {
        LOG_F(ERROR,
              "moveImageToUSB: Specified path is not a valid filesystem or not "
              "ready");
        return;
    }

    if ((fs::status(usbMountPoint).permissions() & fs::perms::owner_write) !=
        fs::perms::none) {
        if (!internal::remountReadWrite(usbMountPoint, PASSWORD)) {
            LOG_F(ERROR,
                  "moveImageToUSB: Failed to remount filesystem as read-write");
            return;
        }
        LOG_F(
            INFO,
            "moveImageToUSB: Filesystem remounted as read-write successfully");
    }

    long long remainingSpace = internal::getUSBSpace(usbMountPoint);
    if (remainingSpace == -1) {
        LOG_F(ERROR, "moveImageToUSB: Failed to get USB space");
        return;
    }

    long long totalSize = internal::getTotalSize(files);
    if (totalSize >= remainingSpace) {
        LOG_F(ERROR, "moveImageToUSB: Insufficient space on USB drive");
        return;
    }

    std::string folderPath = usbMountPoint + "/QUARCS_ImageSave";
    int sumMoveImage = 0;

    for (const auto& imgPath : files) {
        fs::path sourcePath(imgPath);
        fs::path destinationPath = fs::path(folderPath) / sourcePath.filename();

        std::ostringstream mkdirCommand;
        mkdirCommand << "echo '" << PASSWORD << "' | sudo -S mkdir -p "
                     << destinationPath.parent_path().string();
        if (system(mkdirCommand.str().c_str()) != 0) {
            LOG_F(ERROR, "moveImageToUSB: Failed to create directory: {}",
                  destinationPath.parent_path().string().c_str());
            continue;
        }

        std::ostringstream cpCommand;
        cpCommand << "echo '" << PASSWORD << "' | sudo -S cp -r "
                  << sourcePath.string() << " " << destinationPath.string();
        if (system(cpCommand.str().c_str()) != 0) {
            LOG_F(ERROR, "moveImageToUSB: Failed to copy file: {} to {}",
                  sourcePath.string().c_str(),
                  destinationPath.string().c_str());
            continue;
        }

        LOG_F(INFO, "moveImageToUSB: Copied file: {} to {}",
              sourcePath.string().c_str(), destinationPath.string().c_str());
        sumMoveImage++;
    }
    LOG_F(INFO, "moveImageToUSB: Total moved images: %d", sumMoveImage);
}

void deleteFile(const std::string& path) {
    LOG_F(INFO, "deleteFile: Entering function with path: {}", path.c_str());

    std::shared_ptr<ConfigManager> configManager;
    GET_OR_CREATE_PTR(configManager, ConfigManager, Constants::CONFIG_MANAGER)
    std::string imageBasePath =
        configManager->getValue("/lithium/image/base_path")
            .value_or("~/images");
    std::vector<std::string> files = internal::parseString(path, imageBasePath);

    for (const auto& file : files) {
        auto opt = atom::io::compareFileAndSelfPermissions(file);
        if (opt) {
            LOG_F(ERROR, "deleteFile: Failed to compare file permissions: {}",
                  file.c_str());
            continue;
        }
        std::string command;
        if (!opt.value()) {
            std::string password = configManager->getValue("/lithium/password")
                                       .value_or("lithium");
            LOG_F(ERROR, "deleteFile: No permission to delete file: {}",
                  file.c_str());
            command = std::format("echo '{}' | sudo -S rm -rf \"{}\"", password,
                                  file);
        } else {
            command = std::format("rm -rf \"{}\"", file);
        }
        LOG_F(INFO, "deleteFile: Using command: {}", command.c_str());

        auto result = atom::system::executeCommandWithStatus(command).second;

        if (result == 0) {
            LOG_F(INFO, "deleteFile: Deleted file: {}", file.c_str());
        } else {
            LOG_F(ERROR, "deleteFile: Failed to delete file: {}", file.c_str());
        }
    }
}

void usbCheck() {
    LOG_F(INFO, "usbCheck: Entering function");

    std::string base = "/media/";
    std::shared_ptr<atom::utils::Env> env;
    GET_OR_CREATE_PTR(env, atom::utils::Env, Constants::ENVIRONMENT)
    std::string username = env->getEnv("USER");
    std::string basePath = base + username;
    std::string usbMountPoint;

    std::shared_ptr<atom::async::MessageBus> messageBusPtr;
    GET_OR_CREATE_PTR(messageBusPtr, atom::async::MessageBus,
                      Constants::MESSAGE_BUS)

    if (!fs::exists(basePath)) {
        LOG_F(ERROR, "usbCheck: Base directory does not exist.");
        return;
    }

    std::vector<std::string> folderList;
    for (const auto& entry : fs::directory_iterator(basePath)) {
        if (entry.is_directory() && entry.path().filename() != "CDROM") {
            folderList.push_back(entry.path().filename().string());
        }
    }

    if (folderList.size() == 1) {
        usbMountPoint = basePath + "/" + folderList.at(0);
        LOG_F(INFO, "usbCheck: USB mount point: {}", usbMountPoint.c_str());
        std::string usbName = folderList.at(0);
        std::string message = "USBCheck";
        auto disks = atom::system::getDiskUsage();
        long long remainingSpace = -1;
        for (const auto& disk : disks) {
            if (disk.first == usbMountPoint) {
                remainingSpace = disk.second;
            }
        }
        if (remainingSpace == -1) {
            LOG_F(ERROR,
                  "usbCheck: Remaining space is -1. Check the USB drive.");
            return;
        }
        message += ":" + usbName + "," + std::to_string(remainingSpace);
        LOG_F(INFO, "usbCheck: {}", message.c_str());
        messageBusPtr->publish("main", message);

    } else if (folderList.empty()) {
        LOG_F(INFO, "usbCheck: No USB drive found.");
        messageBusPtr->publish("main", "USBCheck:Null, Null");

    } else {
        LOG_F(INFO, "usbCheck: Multiple USB drives found.");
        messageBusPtr->publish("main", "USBCheck:Multiple, Multiple");
    }
}

}  // namespace lithium::middleware