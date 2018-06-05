#ifndef SIKRADIO_UDPSOCKET_HPP
#define SIKRADIO_UDPSOCKET_HPP

#include <Reactor/InputStreamResource.hpp>
#include "../Reactor/DescriptorResource.hpp"
#include "../Reactor/Event.hpp"
#include "Address.hpp"



namespace Utility::Network {

class UDPSocket : public Utility::Reactor::InputStreamResource
{
public:
    UDPSocket();
    virtual ~UDPSocket();

    void bind_address(Address address);
    bool receive(char *buffer, size_t max_len, size_t &rd_len, Address &remote_addr);
    bool receive(char *buffer, size_t max_len, size_t &rd_len);
    size_t send(const char *data, size_t length, const Address &destination);

    void enable_broadcast();
    void join_multicast(const Address &group_address);
    void leave_multicast(const Address &group_address);
    void set_multicast_ttl(int ttl);
};


}

#endif //SIKRADIO_UDPSOCKET_HPP
