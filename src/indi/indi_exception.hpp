#include <exception>
#include <string>

class ConnectionError : public std::exception
{
public:
    ConnectionError(const std::string& error_message) : error_message_(error_message) {}

    virtual const char* what() const noexcept override
    {
        return error_message_.c_str();
    }

private:
    std::string error_message_;
};
