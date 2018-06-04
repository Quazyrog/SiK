#ifndef SIKRADIO_UDPSOCKET_HPP
#define SIKRADIO_UDPSOCKET_HPP

#include "../Reactor/DescriptorResource.hpp"
#include "../Reactor/Event.hpp"
#include "Address.hpp"



namespace Utility::Network {

class UDPSocket : public Utility::Reactor::DescriptorResource
{
protected:
    int fd_;

public:
    UDPSocket();
    ~UDPSocket();

    // Inherited from DescriptorResource
    int descriptor() const override;
    virtual uint32_t event_mask() const override;
    virtual std::shared_ptr<Utility::Reactor::Event> generate_event(uint32_t event_mask, ResourceAction &action) override;
    // [END] Inherited from DescriptorResource

    void bind_address(Address address);
    size_t receive(char *buffer, size_t max_len, Address &remote_addr);
    size_t receive(char *buffer, size_t max_len);
    size_t send(const char *data, size_t length, const Address &destination);
};


class UDPSocketEvent : public Utility::Reactor::Event
{
    UDPSocket *source_;

public:
    explicit UDPSocketEvent(UDPSocket *source);

    /**
     * Return the <c>UDPSocket</c> that caused the event.
     */
    UDPSocket *source() const;

    /**
     * Reenable the resource that caused the event.
     */
    void reenable_source();
};

}

#endif //SIKRADIO_UDPSOCKET_HPP
