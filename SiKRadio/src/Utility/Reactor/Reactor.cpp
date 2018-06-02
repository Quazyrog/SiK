#include <sys/epoll.h>
#include <unistd.h>

#include <regex>

#include "Reactor.hpp"
#include "../Exceptions.hpp"

using Utility::Exceptions::SystemError;

namespace Utility::Reactor {

Reactor::Reactor()
{
    epoll_ = epoll_create1(0);
    if (epoll_ < 0)
        throw SystemError("Unable to create epool");
}


void Reactor::add_descriptor_resource(const std::string &event_name, std::shared_ptr<DescriptorResource> resource)
{
    epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = resource->descriptor();
    if (epoll_ctl(epoll_, EPOLL_CTL_ADD, resource->descriptor(), &ev) != 0)
        throw SystemError("Failed to add resource " + event_name + " to epoll");
}


void Reactor::operator()()
{
    running_ = true;
    while (running_) {
        epoll_event ev;
        epoll_wait(epoll_, &ev, 1, -1);
        uint64_t npass;
        read(ev.data.fd, &npass, sizeof(npass));
        usleep(100'000);
    }
}


void Reactor::stop()
{
    running_ = false;
}


bool Reactor::validate_event_name(const std::string &name, bool allow_wildcard)
{
    std::regex regex;
    if (allow_wildcard)
        regex = std::regex("^[[:alpha:]_][[:alnum:]_]*(.[[:alpha:]_][[:alnum:]_]*)*$");
    else
        regex = std::regex("^([[:alpha:]_][[:alnum:]_]*|\\*)(.([[:alpha:]_][[:alnum:]_]*|\\*))*$");
    return std::regex_match(name, regex);
}

}
