#include <netdb.h>
#include <stdexcept>
#include <arpa/inet.h>
#include "Address.hpp"



namespace Utility::Network {

Address::Address(sockaddr_in addr):
    init_flag_(3),
    addr_(addr)
{}


Address::Address()
{}


Address::Address(const std::string &host):
    Address(host, 0)
{}

Address::Address(const std::string &host, uint16_t port)
{
    addr_.sin_family = AF_INET;
    this->host(host);
    this->port(port);
}


Address::operator sockaddr() const
{
    if ((init_flag_ & 0b11u) != 0b11)
        throw std::logic_error("Address uninitialised");
    sockaddr result;
    std::memcpy(&result, &addr_, sizeof(addr_));
    return result;
}


void Address::host(std::string host)
{
    addrinfo addr_hints;
    addrinfo *addr_result;
    addr_hints.ai_flags = 0;
    addr_hints.ai_family = AF_INET;
    addr_hints.ai_socktype = 0;
    addr_hints.ai_protocol = 0;
    addr_hints.ai_addrlen = 0;
    addr_hints.ai_addr = nullptr;
    addr_hints.ai_canonname = nullptr;
    addr_hints.ai_next = nullptr;
    if (getaddrinfo(host.c_str(), nullptr, &addr_hints, &addr_result) != 0)
        throw std::invalid_argument("Invalid address specification");

    addr_.sin_addr.s_addr = reinterpret_cast<struct sockaddr_in *>(addr_result->ai_addr)->sin_addr.s_addr;

    freeaddrinfo(addr_result);

    init_flag_ |= 2;
}


std::string Address::host() const
{
    return inet_ntoa(addr_.sin_addr);
}


void Address::port(uint16_t port)
{
    addr_.sin_port = htons(port);
    init_flag_ |= 1;
}


uint16_t Address::port() const
{
    return ntohs(addr_.sin_port);
}


Address Address::localhost(uint16_t port)
{
    Address a;
    a.addr_.sin_family = AF_INET;
    a.addr_.sin_addr.s_addr = htonl(INADDR_ANY);
    a.addr_.sin_port = htons(port);
    a.init_flag_ = 3;
    return a;
}


Address::operator std::string() const
{
    return host() + ":" + std::to_string(port());
}


bool Address::operator!=(const Address &other) const
{
    return !(*this == other);
}


bool Address::operator==(const Address &other) const
{
    return other.port() == port() && other.host() == host() && other.init_flag_ == init_flag_;
}


std::ostream &operator<<(std::ostream &ostream, const Address &addr)
{
    return (ostream << static_cast<std::string>(addr));
}

}
