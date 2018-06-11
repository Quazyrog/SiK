#ifndef SIKRADIO_CONTROLCOMPONENT_HPP
#define SIKRADIO_CONTROLCOMPONENT_HPP

#include <cstdint>
#include <set>
#include "TransmitterMisc.hpp"



class ControlComponent : public Utility::Reactor::EventListener
{
protected:
    Utility::Misc::LoggerType logger_;

    Utility::Reactor::Reactor &reactor_;
    std::shared_ptr<Utility::Network::UDPSocket> socket_;

    std::string name_;
    Utility::Network::Address mcast_addr_;

    std::unordered_set<uint64_t> retrans_packets_;

    void handle_event_(std::shared_ptr<Utility::Reactor::Event> event) override;
    void handle_data_(std::shared_ptr<Utility::Reactor::StreamEvent> event);
    void execute_command_(std::stringstream ss, Utility::Network::Address from_address);

public:
    ControlComponent(const Utility::Misc::Params &params, Utility::Reactor::Reactor &reactor, Utility::Misc::LoggerType logger);
};



#endif //SIKRADIO_CONTROLCOMPONENT_HPP
