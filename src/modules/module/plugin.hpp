#pragma once

#include <string>

class Plugin
{
public:
    Plugin(std::string path, std::string version, std::string author, std::string description);

    const std::string &GetPath() const;
    const std::string &GetVersion() const;
    const std::string &GetAuthor() const;
    const std::string &GetDescription() const;

private:
    std::string path_;
    std::string version_;
    std::string author_;
    std::string description_;
};