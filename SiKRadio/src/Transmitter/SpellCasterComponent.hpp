#ifndef SIKRADIO_SPELLCASTERCOMPONENT_HPP
#define SIKRADIO_SPELLCASTERCOMPONENT_HPP

#include <Reactor/EventListener.hpp>
#include <Reactor/Reactor.hpp>
#include <Misc.hpp>
#include <cstdint>
#include <Reactor/StreamResource.hpp>
#include <Network/UDPSocket.hpp>
#include "TransmitterMisc.hpp"
#include "AudioFIFOBuffer.hpp"
#include "Events.hpp"



class SpellCasterComponent : public Utility::Reactor::EventListener
{
protected:
    const size_t package_size_;
    const size_t buffer_capacity_;

    Utility::Reactor::Reactor &reactor_;
    Utility::Misc::LoggerType logger_;
    std::shared_ptr<Utility::Reactor::IStreamResource> stdin_;
    std::shared_ptr<Utility::Network::UDPSocket> socket_;
    Utility::Network::Address mcast_addr_;

    AudioFIFOBuffer fifo_;
    char *buffer_;
    size_t buffer_fill_ = 0;

    void handle_event_(std::shared_ptr<Utility::Reactor::Event> event) override;
    void handle_stdin_(std::shared_ptr<Utility::Reactor::StreamEvent> event);

    void do_retransmission_(std::shared_ptr<RetransmissionEvent> event);

public:
    SpellCasterComponent(const Utility::Misc::Params &params, Utility::Reactor::Reactor &reactor,
                         Utility::Misc::LoggerType logger);
    ~SpellCasterComponent();
};



#endif //SIKRADIO_SPELLCASTERCOMPONENT_HPP
