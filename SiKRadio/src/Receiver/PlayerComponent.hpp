#ifndef SIKRADIO_PLAYERCOMPONENT_HPP
#define SIKRADIO_PLAYERCOMPONENT_HPP

#include <Reactor/EventListener.hpp>
#include <Network/UDPSocket.hpp>
#include <Network/Address.hpp>
#include <Misc.hpp>
#include <AudioPacketBuffer.hpp>
#include "ReceiverMisc.hpp"
#include "Events.hpp"



class PlayerComponent : public Utility::Reactor::EventListener
{
protected:
    enum State {WAIT_FIRST_DATA, WAIT_BUFFER, STREAM};


    Utility::Reactor::Reactor &reactor_;
    std::shared_ptr<Utility::Network::UDPSocket> socket_;

    std::string station_name_;
    Utility::Network::Address local_address_;
    Utility::Network::Address station_address_;

    std::shared_ptr<Utility::Reactor::OStreamResource> stdout_;
    Utility::AudioPacketBuffer buffer_;
    State state_ = WAIT_FIRST_DATA;
    bool stdout_ready_ = false;
    Utility::AudioPacket in_packet_;
    size_t part_packet_read_ = 0;
    Utility::AudioPacket out_packet_;
    size_t part_packet_write_ = 0;

    void handle_event_(std::shared_ptr<Utility::Reactor::Event> event) override;
    void handle_data_(std::shared_ptr<Utility::Reactor::StreamEvent> event);
    void handle_station_event_(std::shared_ptr<Events::Lookup::StationEvent> event);

    void try_write_();

public:
    PlayerComponent(const Utility::Misc::Params &params, Utility::Reactor::Reactor &reactor);
    virtual ~PlayerComponent() = default;

    void play_station(std::string name);
};



#endif //SIKRADIO_PLAYERCOMPONENT_HPP
