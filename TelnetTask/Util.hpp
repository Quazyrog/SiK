#ifndef TELNETTASK_UTIL_HPP
#define TELNETTASK_UTIL_HPP

#include <exception>
#include <stdexcept>
#include <cerrno>
#include <cstring>



class SystemError : public std::runtime_error
{
private:
    const decltype(errno) errno_;

public:
    SystemError():
        std::runtime_error(strerror(errno)),
        errno_(errno)
    {}


    decltype(errno) errorCode() const
    {
        return errno_;
    }
};


class IOError : public std::runtime_error
{
public:
    explicit IOError(const std::string &msg) : runtime_error(msg)
    {}

    IOError(const char *msg) : runtime_error(msg)
    {}
};

#endif //TELNETTASK_UTIL_HPP
