#pragma once

#include "hdu.hpp"
#include <vector>
#include <string>

class FITSFile {
public:
    void readFITS(const std::string& filename);
    void writeFITS(const std::string& filename) const;

    size_t getHDUCount() const;
    const HDU& getHDU(size_t index) const;
    HDU& getHDU(size_t index);
    void addHDU(std::unique_ptr<HDU> hdu);

private:
    std::vector<std::unique_ptr<HDU>> hdus;
};
