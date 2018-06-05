#include <iostream>
#include <Reactor/Reactor.hpp>
#include "PlayerComponent.hpp"



PlayerComponent::PlayerComponent(const Utility::Misc::Params &params, Utility::Reactor::Reactor &reactor):
    reactor_(reactor)
{
    socket_.make_nonblocking();
    socket_.enable_broadcast();
    socket_.bind_address(Utility::Network::Address::localhost(params.data_port));

    add_filter_("/Lookup/Station/.*");
}


void PlayerComponent::play_station(std::string name)
{
    if (name.empty() || name.length() > 64)
        throw std::invalid_argument("Invalid station name");
    wait_for_buffer_to_fill_ = true;
    station_name_ = name;
    std::cerr << "PlayerComponent: now will play '" << name << "'" << std::endl;
}


void PlayerComponent::handle_event_(std::shared_ptr<Utility::Reactor::Event> event)
{
    if (event->name().substr(0, 16) == "/Lookup/Station/") {
        auto ev = std::dynamic_pointer_cast<StationEvent>(event);
        handle_station_event_(ev);
    }
}


void PlayerComponent::handle_station_event_(std::shared_ptr<StationEvent> event)
{
    auto bad_ev = std::dynamic_pointer_cast<StationTimedOutEvent>(event);
    if (bad_ev != nullptr) {
        // Station removed
        if (bad_ev->station_data().name == station_name_) {
            // Wombat is sad; wombat lost connection
            station_name_ = "";
            reactor_.broadcast_event(std::make_shared<ConnectionLostEvent>());
            socket_.leave_multicast(station_address_);
            station_address_ = Utility::Network::Address();
            std::cerr << "PlayerComponent: playied station timed out" << std::endl;
        }

    } else {
        // Station added or changed
        if (station_name_.empty())
            play_station(event->station_data().name); /* in either case we want to connect to it */

        if (station_name_ == event->station_data().name) {
            // It is our station that we want to update
            if (!station_address_.empty())
                socket_.leave_multicast(station_address_);
            station_address_ = Utility::Network::Address(event->station_data().mcast_addr.host());
            socket_.join_multicast(station_address_);
            std::cerr << "PlayerComponent: changed playied station addres to " << station_address_ << std::endl;
        }
    }
}
