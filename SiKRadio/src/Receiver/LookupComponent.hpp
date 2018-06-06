#ifndef SIKRADIO_LOOKUPCOMPONENT_HPP
#define SIKRADIO_LOOKUPCOMPONENT_HPP

#include <Reactor/EventListener.hpp>
#include <Misc.hpp>
#include <Reactor/Reactor.hpp>
#include <Network/UDPSocket.hpp>
#include <Reactor/Timer.hpp>
#include "ReceiverMisc.hpp"



class LookupComponent : public Utility::Reactor::EventListener
{
protected:
    Utility::Reactor::Reactor &reactor_;
    std::shared_ptr<Utility::Network::UDPSocket> ctrl_socket_;
    std::shared_ptr<Utility::Reactor::Timer> lookup_timer_;

    Utility::Network::Address discovery_addr_;

    std::map<std::string, StationData> stations_;
    bool player_wants_station_ = false;

    void handle_event_(std::shared_ptr<Utility::Reactor::Event> event) override;

    void receive_ctrl_command_();
    void send_lookup_();
    void execute_ctrl_command_(std::stringstream command, Utility::Network::Address from);

    void station_gc_();

public:
    explicit LookupComponent(const Utility::Misc::Params &params, Utility::Reactor::Reactor &reactor);
    virtual ~LookupComponent() = default;
};



#endif //SIKRADIO_LOOKUPCOMPONENT_HPP
