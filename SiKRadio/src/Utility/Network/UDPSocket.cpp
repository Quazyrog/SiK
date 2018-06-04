#include <sys/socket.h>
#include <sys/epoll.h>
#include <unistd.h>
#include "../Reactor/Reactor.hpp"
#include "../Exceptions.hpp"
#include "UDPSocket.hpp"

namespace Utility::Network {

UDPSocket::UDPSocket()
{
    fd_ = socket(PF_INET, SOCK_DGRAM, 0);
    if (fd_ < 0)
        throw Utility::Exceptions::SystemError("Unable to create socket");
}


void UDPSocket::bind_address(Address address)
{
    sockaddr addr = static_cast<sockaddr>(address);
    if (::bind(fd_, &addr, sizeof(addr)) < 0)
        throw Utility::Exceptions::SystemError("Failed to bind socket");
}


size_t UDPSocket::receive(char *buffer, size_t max_len)
{
    Address a;
    return receive(buffer, max_len, a);
}


size_t UDPSocket::receive(char *buffer, size_t max_len, Address &remote_addr)
{
    sockaddr_in raddr;
    socklen_t raddr_len = sizeof(raddr);
    auto rcv_len = recvfrom(fd_, buffer, max_len, 0, reinterpret_cast<struct sockaddr *>(&raddr), &raddr_len);
    if (rcv_len < 0)
        throw Utility::Exceptions::IOError("Receive from socket failed");
    remote_addr = Address(raddr);
    return static_cast<size_t>(rcv_len);
}


int UDPSocket::descriptor() const
{
    return fd_;
}


uint32_t UDPSocket::event_mask() const
{
    return EPOLLIN;
}


std::shared_ptr<Utility::Reactor::Event>
UDPSocket::generate_event(uint32_t event_mask, Reactor::DescriptorResource::ResourceAction &action)
{
    if (!(event_mask & EPOLLIN)) {
        action = DO_NOTHING;
        return nullptr;
    }
    action = SUSPEND;
    return std::make_shared<UDPSocketEvent>(this);
}


UDPSocket::~UDPSocket()
{
    close(fd_);
}


size_t UDPSocket::send(const char *data, size_t length, const Address &destination)
{
    auto addr = static_cast<sockaddr>(destination);
    auto snd_len = sendto(fd_, data, length, 0, &addr, sizeof(addr));
    if (snd_len < 0)
        throw Utility::Exceptions::IOError("Cannot sent data with socket");
    return static_cast<size_t>(snd_len);
}


UDPSocketEvent::UDPSocketEvent(UDPSocket *source) :
    Event(source->bound_name()),
    source_(source)
{}


UDPSocket *UDPSocketEvent::source() const
{
    return source_;
}


void UDPSocketEvent::reenable_source()
{
    source_->bound_reactor()->reenable_resource(source_);
}

}
