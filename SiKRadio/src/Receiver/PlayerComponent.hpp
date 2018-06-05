#ifndef SIKRADIO_PLAYERCOMPONENT_HPP
#define SIKRADIO_PLAYERCOMPONENT_HPP

#include <Reactor/EventListener.hpp>
#include <Network/UDPSocket.hpp>
#include <Network/Address.hpp>
#include <Misc.hpp>
#include "ReceiverMisc.hpp"



class PlayerComponent : public Utility::Reactor::EventListener
{
protected:
    Utility::Reactor::Reactor &reactor_;
    Utility::Network::UDPSocket socket_;

    std::string station_name_;
    Utility::Network::Address station_address_;
    bool wait_for_buffer_to_fill_ = true;

    void handle_event_(std::shared_ptr<Utility::Reactor::Event> event) override;

    void handle_station_event_(std::shared_ptr<StationEvent> event);

public:
    PlayerComponent(const Utility::Misc::Params &params, Utility::Reactor::Reactor &reactor);
    virtual ~PlayerComponent() = default;

    void play_station(std::string name);
};



#endif //SIKRADIO_PLAYERCOMPONENT_HPP
