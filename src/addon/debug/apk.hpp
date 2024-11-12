#ifndef LITHIUM_ADDON_DEBUG_APK_HPP
#define LITHIUM_ADDON_DEBUG_APK_HPP

#include <archive.h>
#include <fstream>
#include <string>
#include <vector>

class APKTool {
public:
    APKTool(const std::string& apkPath, const std::string& outputDir);
    ~APKTool();

    void extract(bool parseManifest = false);
    void repack();
    void optimizeResources();
    void analyzeObfuscation();
    void analyzeDependencies();
    void scanVulnerabilities();
    void performanceAnalysis();
    void signAPK(const std::string& keystore, const std::string& alias,
                 const std::string& keystorePassword);
    void verifySignature();

private:
    std::string apkPath;
    std::string outputDir;
    std::string logFile;
    std::ofstream logStream;

    void parseManifestFile(const std::string& manifestPath);
    void writeFileList(const std::vector<std::string>& fileList);
    void log(const std::string& message);
};

#endif  // LITHIUM_ADDON_DEBUG_APK_HPP