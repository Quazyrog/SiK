#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include "../Exceptions.hpp"
#include "InputStreamResource.hpp"
#include "Reactor.hpp"



namespace Utility::Reactor {

namespace
{
    class StdinResource : public InputStreamResource
    {
    public:
        StdinResource():
            InputStreamResource(0)
        {}
    };
}


std::atomic<class InputStreamResource*> stdin_resource_{nullptr};


InputStreamResource::InputStreamResource(int fd):
    fd_(fd)
{}


int InputStreamResource::descriptor() const
{
    return fd_;
}


uint32_t InputStreamResource::event_mask() const
{
    return EPOLLIN;
}


std::shared_ptr<Event> InputStreamResource::generate_event(uint32_t event_mask,
    DescriptorResource::ResourceAction &action)
{
    if (event_mask & EPOLLIN) {
        action = SUSPEND;
        return std::make_shared<InputStreamEvent>(this);
    }
    action = DO_NOTHING;
    return nullptr;
}


bool InputStreamResource::read(char *buf, const size_t max_len, size_t &rd_len)
{
    auto result = ::read(fd_, buf, max_len);
    if (result < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return false;
        throw Utility::Exceptions::IOError("read(" + std::to_string(fd_) + ", ...) failed");
    }
    rd_len = static_cast<size_t>(result);
    return true;
}


std::shared_ptr<class InputStreamResource> InputStreamResource::stdin_resource()
{
    return std::make_shared<StdinResource>();
}


InputStreamEvent::InputStreamEvent(InputStreamResource *resource) :
        Event(resource->bound_name()),
        source_(resource)
{}


InputStreamResource *InputStreamEvent::source()
{
    return source_;
}


void InputStreamResource::make_nonblocking()
{
    if(fcntl(fd_, F_SETFL, fcntl(fd_, F_GETFL) | O_NONBLOCK) < 0)
        throw Utility::Exceptions::SystemError("Cannot switch to non-blocking mode");
}


void InputStreamEvent::reenable_source()
{
    source_->bound_reactor()->reenable_resource(source_);
}

}
