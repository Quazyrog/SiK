#ifndef SIKRADIO_ADDRESS_HPP
#define SIKRADIO_ADDRESS_HPP

#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <string>



namespace Utility::Network {

class Address
{
    friend class UDPSocket;
    unsigned char init_flag_ = 0;
    sockaddr_in addr_;

protected:
    explicit Address(sockaddr_in);

public:
    static Address localhost(uint16_t port);

    Address();
    Address(const std::string &host, uint16_t port);

    void host(std::string host);
    std::string host() const;
    void port(uint16_t port);
    uint16_t port() const;

    explicit operator std::string() const;
    explicit operator sockaddr() const;
};

}

#endif //SIKRADIO_ADDRESS_HPP
