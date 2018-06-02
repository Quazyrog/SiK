#ifndef SIKRADIO_EXCEPTIONS_HPP
#define SIKRADIO_EXCEPTIONS_HPP

#include <stdexcept>
#include <cstring>



namespace Utility::Exceptions {

class SystemError : public std::runtime_error
{
public:
    SystemError(const std::string &msg):
        runtime_error(msg + "(errno " + std::to_string(errno) + ": " + strerror(errno) + ")")
    {}

    SystemError(const std::string &msg, int err_no):
            runtime_error(msg + "(errno " + std::to_string(err_no) + ": " + strerror(err_no) + ")")
    {}
};

}

#endif //SIKRADIO_EXCEPTIONS_HPP
