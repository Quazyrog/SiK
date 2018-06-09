#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include "../Exceptions.hpp"
#include "StreamResource.hpp"
#include "Reactor.hpp"



namespace Utility::Reactor {

namespace {
class StdinResource : public virtual IStreamResource
{
public:
    StdinResource():
        StreamResource(0),
        IStreamResource(0)
    {}
    virtual ~StdinResource() = default;
};


class StdoutResource : public virtual OStreamResource
{
public:
    StdoutResource():
        StreamResource(1),
        OStreamResource(1)
    {}
    virtual ~StdoutResource() = default;
};
}


std::shared_ptr<class IStreamResource> StreamResource::stdin_resource()
{
    return std::make_shared<StdinResource>();
}


std::shared_ptr<class OStreamResource> StreamResource::stdout_resource()
{
    return std::make_shared<StdoutResource>();
}


StreamResource::StreamResource(int fd):
        fd_(fd)
{}


void StreamResource::make_nonblocking()
{
    if (fcntl(fd_, F_SETFL, fcntl(fd_, F_GETFL) | O_NONBLOCK) < 0)
        throw Utility::Exceptions::SystemError("Cannot switch to non-blocking mode");
}


int StreamResource::descriptor() const
{
    return fd_;
}


std::shared_ptr<Event> StreamResource::generate_event(uint32_t event_mask, DescriptorResource::ResourceAction &action)
{
    if (event_mask & EPOLLIN) {
        action = SUSPEND;
        return std::make_shared<StreamEvent>(this, StreamEvent::CAN_READ);
    }
    if (event_mask & EPOLLOUT) {
        action = SUSPEND;
        return std::make_shared<StreamEvent>(this, StreamEvent::CAN_WRITE);
    }
    action = DO_NOTHING;
    return nullptr;
}


IStreamResource::IStreamResource(int fd) :
        StreamResource(fd)
{}


uint32_t IStreamResource::event_mask() const
{
    return EPOLLIN;
}


bool IStreamResource::read(char *buf, const size_t max_len, size_t &rd_len)
{
    auto result = ::read(fd_, buf, max_len);
    if (result < 0) {
        rd_len = 0;
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return false;
        throw Utility::Exceptions::IOError("read(" + std::to_string(fd_) + ", ...) failed");
    }
    rd_len = static_cast<size_t>(result);
    return true;
}


OStreamResource::OStreamResource(int fd):
    StreamResource(fd)
{}


uint32_t OStreamResource::event_mask() const
{
    return EPOLLOUT;
}


bool OStreamResource::write(char *data, size_t len, size_t &wr_len)
{
    auto result = ::write(fd_, data, len);
    if (result < 0) {
        wr_len = 0;
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return false;
        throw Utility::Exceptions::IOError("write(" + std::to_string(fd_) + ", ...) failed");
    }
    wr_len = static_cast<size_t>(result);
    return true;
}


uint32_t IOStreamResource::event_mask() const
{
    return EPOLLIN | EPOLLOUT;
}


IOStreamResource::IOStreamResource(int fd):
    StreamResource(fd)
{}


StreamEvent::StreamEvent(StreamResource *resource, EventType et) :
        Event(resource->bound_name()),
        source_(resource),
        type_(et)
{}


StreamResource *StreamEvent::source()
{
    return source_;
}


void StreamEvent::reenable_source()
{
    source_->bound_reactor()->reenable_resource(source_);
}


StreamEvent::EventType StreamEvent::type() const
{
    return type_;
}

}
