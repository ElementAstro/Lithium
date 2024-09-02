#include <fstream>
#include <iostream>

#include "atom/io/compress.hpp"

// Creates a sample text file to compress
void createSampleFile(const std::string& fileName) {
    std::ofstream outFile(fileName);
    if (outFile) {
        outFile << "This is a sample text file for compression testing.";
        outFile.close();
        std::cout << "Created sample file: " << fileName << std::endl;
    } else {
        std::cerr << "Failed to create file: " << fileName << std::endl;
    }
}

int main() {
    const std::string sampleFile = "testfile.txt";
    const std::string outputFolder = ".";  // Use current directory
    const std::string zipFile = "testarchive.zip";

    // Step 1: Create a sample file
    createSampleFile(sampleFile);

    // Step 2: Compress the sample file using Gzip
    if (atom::io::compressFile(sampleFile, outputFolder)) {
        std::cout << "Successfully compressed " << sampleFile << std::endl;
    } else {
        std::cerr << "Failed to compress " << sampleFile << std::endl;
    }

    // Step 3: Create a ZIP file containing the sample file
    if (atom::io::createZip(outputFolder, zipFile)) {
        std::cout << "Successfully created ZIP file: " << zipFile << std::endl;
    } else {
        std::cerr << "Failed to create ZIP file: " << zipFile << std::endl;
    }

    // Step 4: List files in the ZIP file
    auto filesInZip = atom::io::listFilesInZip(zipFile);
    std::cout << "Files in ZIP archive (" << zipFile << "):" << std::endl;
    for (const auto& file : filesInZip) {
        std::cout << " - " << file << std::endl;
    }

    // Step 5: Check if the sample file exists in the ZIP
    if (atom::io::fileExistsInZip(zipFile, sampleFile)) {
        std::cout << sampleFile << " exists in " << zipFile << std::endl;
    } else {
        std::cout << sampleFile << " does not exist in " << zipFile
                  << std::endl;
    }

    // Step 6: Get the size of the file in the ZIP
    size_t fileSize = atom::io::getZipFileSize(zipFile);
    std::cout << "Size of file in ZIP: " << fileSize << " bytes" << std::endl;

    // Step 7: Remove the file from the ZIP
    if (atom::io::removeFileFromZip(zipFile, sampleFile)) {
        std::cout << "Removed " << sampleFile << " from " << zipFile
                  << std::endl;
    } else {
        std::cerr << "Failed to remove " << sampleFile << " from " << zipFile
                  << std::endl;
    }

    // Step 8: Extract the ZIP file (not shown here for brevity)
    // Uncomment the following to extract:
    // if (atom::io::extractZip(zipFile, outputFolder)) {
    //     std::cout << "Successfully extracted " << zipFile << " to " <<
    //     outputFolder << std::endl;
    // } else {
    //     std::cerr << "Failed to extract " << zipFile << std::endl;
    // }

    return 0;
}
