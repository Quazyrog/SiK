#ifndef SIKRADIO_CONTROLCOMPONENT_HPP
#define SIKRADIO_CONTROLCOMPONENT_HPP

#include <cstdint>
#include <set>



class ControlComponent : public Utility::Reactor::EventListener
{
    Utility::Reactor::Reactor &reactor_;
    std::shared_ptr<Utility::Network::UDPSocket> socket_;

    std::string name_;
    Utility::Network::Address mcastAddr_;

    std::unordered_set<uint64_t> retransPackets_;

public:
    ControlComponent(const Utility::Misc::Params &params, Utility::Reactor::Reactor &reactor);

protected:
    void handle_event_(std::shared_ptr<Utility::Reactor::Event> event) override;
    void handle_data_(std::shared_ptr<Utility::Reactor::StreamEvent> event);
    void execute_command_(std::stringstream ss, Utility::Network::Address from_address);
};



#endif //SIKRADIO_CONTROLCOMPONENT_HPP
