#include <sys/epoll.h>
#include <unistd.h>

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
    std::lock_guard lock_guard(descriptor_resources_lock_);

    if (descriptor_resources_.find(event_name) != descriptor_resources_.end())
        throw std::invalid_argument("Resource with event's name `" + event_name + "` already present");

    epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = resource->descriptor();
    if (epoll_ctl(epoll_, EPOLL_CTL_ADD, resource->descriptor(), &ev) != 0)
        throw SystemError("Failed to add resource " + event_name + " to epoll");

    descriptor_resources_[event_name] = resource;
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

}
