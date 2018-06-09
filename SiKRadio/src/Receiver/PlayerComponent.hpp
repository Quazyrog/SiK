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


    Utility::Misc::LoggerType logger_;
    Utility::Reactor::Reactor &reactor_;
    std::shared_ptr<Utility::Network::UDPSocket> socket_;
    std::shared_ptr<Utility::Reactor::Timer> timer_;

    std::string station_name_; // fixme just station data
    Utility::Network::Address local_address_;
    Utility::Network::Address station_address_;
    Utility::Network::Address station_ctrl_addr_;

    std::shared_ptr<Utility::Reactor::OStreamResource> stdout_;
    Utility::AudioPacketBuffer buffer_;
    State state_ = WAIT_FIRST_DATA;
    bool stdout_ready_ = false;

    void handle_event_(std::shared_ptr<Utility::Reactor::Event> event) override;
    void handle_data_(std::shared_ptr<Utility::Reactor::StreamEvent> event);
    void handle_station_event_(std::shared_ptr<Events::Lookup::StationEvent> event);

    void try_write_();

public:
    PlayerComponent(const Utility::Misc::Params &params, Utility::Reactor::Reactor &reactor,
                    Utility::Misc::LoggerType logger);
    virtual ~PlayerComponent() = default;

    void play_station(std::string name);
};



#endif //SIKRADIO_PLAYERCOMPONENT_HPP
