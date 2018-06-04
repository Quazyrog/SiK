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


void UDPSocket::join_multicast(const Address &group_address)
{
    ip_mreq ip_mreq;
    ip_mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    ip_mreq.imr_multiaddr = group_address.addr_.sin_addr;
    if (setsockopt(fd_, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*)&ip_mreq, sizeof ip_mreq) < 0)
        throw Utility::Exceptions::SystemError("Failed to join multi-cast group");
}

}
