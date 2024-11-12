#include "apk.hpp"

#include <archive.h>
#include <archive_entry.h>

#include <filesystem>
#include <thread>

#include "atom/log/loguru.hpp"

namespace fs = std::filesystem;

const int PACKAGE_NAME_OFFSET = 9;
const int VERSION_NAME_OFFSET = 21;
const int PERMISSION_OFFSET = 30;
const int ARCHIVE_BLOCK_SIZE = 10240;

APKTool::APKTool(const std::string& apkPath, const std::string& outputDir)
    : apkPath(apkPath),
      outputDir(outputDir),
      logFile(outputDir + "/apktool.log") {
    fs::create_directories(outputDir);
    loguru::add_file(logFile.c_str(), loguru::Append, loguru::Verbosity_MAX);
    LOG_F(INFO,
          "APKTool initialized with APK path: {} and output directory: {}",
          apkPath, outputDir);
}

APKTool::~APKTool() { LOG_F(INFO, "APKTool instance destroyed."); }

void APKTool::extract(bool parseManifest) {
    LOG_F(INFO, "Starting to extract APK file.");
    struct archive* archivePtr = archive_read_new();
    struct archive_entry* entry;
    int result;

    archive_read_support_format_zip(archivePtr);

    result = archive_read_open_filename(archivePtr, apkPath.c_str(),
                                        ARCHIVE_BLOCK_SIZE);
    if (result) {
        LOG_F(ERROR, "Failed to open APK file: {}, error code: {}", apkPath,
              result);
        return;
    }

    std::vector<std::string> fileList;
    std::vector<std::thread> threads;
    std::mutex mtx;

    while (archive_read_next_header(archivePtr, &entry) == ARCHIVE_OK) {
        std::string entryName = archive_entry_pathname(entry);
        fileList.push_back(entryName);

        threads.emplace_back([this, entryName, &archivePtr, &mtx]() {
            std::string outputFilePath =
                (fs::path(outputDir) / entryName).string();
            fs::create_directories(fs::path(outputFilePath).parent_path());

            std::ofstream outFile(outputFilePath, std::ios::binary);
            const void* buff;
            size_t size;
            la_int64_t offset;

            while (true) {
                int result =
                    archive_read_data_block(archivePtr, &buff, &size, &offset);
                if (result == ARCHIVE_EOF)
                    break;
                if (result < ARCHIVE_OK) {
                    std::lock_guard<std::mutex> lock(mtx);
                    LOG_F(ERROR, "Failed to extract file: {}, error code: {}",
                          entryName, result);
                    return;
                }
                outFile.write(static_cast<const char*>(buff),
                              static_cast<std::streamsize>(size));
            }

            outFile.close();
            std::lock_guard<std::mutex> lock(mtx);
            LOG_F(INFO, "Successfully extracted file: {}", entryName);
        });

        archive_read_data_skip(archivePtr);
    }

    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    if (parseManifest) {
        parseManifestFile(fs::path(outputDir) / "AndroidManifest.xml");
    }

    writeFileList(fileList);
    archive_read_free(archivePtr);
    LOG_F(INFO, "Extraction completed.");
}

void APKTool::repack() {
    LOG_F(INFO, "Starting to repack APK.");
    std::string command =
        "apktool b " + outputDir + " -o " + outputDir + "/output.apk";
    std::system(command.c_str());
    LOG_F(INFO, "APK repacking completed: {}/output.apk", outputDir);
}

void APKTool::optimizeResources() {
    LOG_F(INFO, "Starting resource optimization.");
    for (const auto& entry : fs::recursive_directory_iterator(outputDir)) {
        if (entry.path().extension() == ".png") {
            std::string cmd = "optipng -o2 " + entry.path().string();
            std::system(cmd.c_str());
            LOG_F(INFO, "Optimized resource: {}", entry.path().string());
        }
    }
    LOG_F(INFO, "Resource optimization completed.");
}

void APKTool::analyzeObfuscation() {
    LOG_F(INFO, "Starting obfuscation analysis.");
    std::string command = "jadx -d " + outputDir + "/jadx_output " + apkPath;
    std::system(command.c_str());
    LOG_F(INFO, "Obfuscation analysis completed.");
}

void APKTool::analyzeDependencies() {
    LOG_F(INFO, "Starting dependency analysis.");
    std::string gradleFile = outputDir + "/build.gradle";
    if (!fs::exists(gradleFile)) {
        LOG_F(ERROR, "build.gradle file not found.");
        return;
    }

    std::ifstream file(gradleFile);
    std::string line;
    std::vector<std::string> dependencies;

    while (std::getline(file, line)) {
        if (line.find("implementation") != std::string::npos) {
            dependencies.push_back(line);
        }
    }

    LOG_F(INFO, "Dependency analysis results:");
    for (const auto& dep : dependencies) {
        LOG_F(INFO, "{}", dep);
    }
}

void APKTool::scanVulnerabilities() {
    LOG_F(INFO, "Starting vulnerability scan.");
    std::string command =
        "dependency-check --project APKTool --scan " + outputDir;
    std::system(command.c_str());
    LOG_F(INFO, "Vulnerability scan completed.");
}

void APKTool::performanceAnalysis() {
    LOG_F(INFO, "Starting performance analysis.");
    std::string command = "adb shell am start -n " + apkPath;
    std::system(command.c_str());
    LOG_F(INFO, "Performance analysis completed.");
}

void APKTool::signAPK(const std::string& keystore, const std::string& alias,
                      const std::string& keystorePassword) {
    LOG_F(INFO, "Starting APK signing.");
    std::string command =
        "jarsigner -verbose -sigalg SHA1withRSA -digestalg SHA1 -keystore " +
        keystore + " -storepass " + keystorePassword + " " + outputDir +
        "/output.apk " + alias;
    std::system(command.c_str());
    LOG_F(INFO, "APK signing completed.");
}

void APKTool::verifySignature() {
    LOG_F(INFO, "Starting APK signature verification.");
    std::string command = "apksigner verify " + outputDir + "/output.apk";
    std::system(command.c_str());
    LOG_F(INFO, "APK signature verification completed.");
}

void APKTool::parseManifestFile(const std::string& manifestPath) {
    LOG_F(INFO, "Parsing AndroidManifest.xml.");
    std::ifstream manifestFile(manifestPath);
    if (!manifestFile) {
        LOG_F(ERROR, "Failed to open AndroidManifest.xml.");
        return;
    }

    std::stringstream buffer;
    buffer << manifestFile.rdbuf();
    std::string manifestContent = buffer.str();
    manifestFile.close();

    std::size_t pos = manifestContent.find("package=\"");
    if (pos != std::string::npos) {
        pos += PACKAGE_NAME_OFFSET;
        std::size_t end = manifestContent.find('\"', pos);
        std::string packageName = manifestContent.substr(pos, end - pos);
        LOG_F(INFO, "Package name: {}", packageName);
    }

    pos = manifestContent.find("android:versionName=\"");
    if (pos != std::string::npos) {
        pos += VERSION_NAME_OFFSET;
        std::size_t end = manifestContent.find('\"', pos);
        std::string versionName = manifestContent.substr(pos, end - pos);
        LOG_F(INFO, "Version name: {}", versionName);
    }

    LOG_F(INFO, "Extracting application permissions:");
    std::size_t current = 0;
    while ((pos = manifestContent.find("<uses-permission android:name=\"",
                                       current)) != std::string::npos) {
        pos += PERMISSION_OFFSET;
        std::size_t end = manifestContent.find('\"', pos);
        std::string permission = manifestContent.substr(pos, end - pos);
        LOG_F(INFO, "Permission: {}", permission);
        current = end;
    }
}

void APKTool::writeFileList(const std::vector<std::string>& fileList) {
    std::ofstream listFile(outputDir + "/file_list.txt");
    for (const auto& fileName : fileList) {
        listFile << fileName << std::endl;
    }
    listFile.close();
    LOG_F(INFO, "File list written.");
}

void APKTool::log(const std::string& message) { LOG_F(INFO, "{}", message); }