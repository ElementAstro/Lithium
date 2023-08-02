#include "plugin.hpp"

Plugin::Plugin(std::string path, std::string version, std::string author, std::string description)
    : path_(std::move(path)), version_(std::move(version)), author_(std::move(author)), description_(std::move(description)) {}

const std::string &Plugin::GetPath() const
{
    return path_;
}

const std::string &Plugin::GetVersion() const
{
    return version_;
}

const std::string &Plugin::GetAuthor() const
{
    return author_;
}

const std::string &Plugin::GetDescription() const
{
    return description_;
}