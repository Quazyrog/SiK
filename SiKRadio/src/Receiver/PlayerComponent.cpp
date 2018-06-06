#include <iostream>
#include <Reactor/Reactor.hpp>
#include "PlayerComponent.hpp"

using namespace Events::Player;



PlayerComponent::PlayerComponent(const Utility::Misc::Params &params, Utility::Reactor::Reactor &reactor):
    reactor_(reactor),
    buffer_(params.bsize, params.psize)
{
    socket_ = std::make_shared<Utility::Network::UDPSocket>();
    socket_->make_nonblocking();
    socket_->enable_broadcast();
    reactor_.add_resource("/Player/Internal/DataAvailable", socket_);

    add_filter_("/Player/Internal/.*");
    add_filter_("/Lookup/Station/.*");
}


void PlayerComponent::play_station(std::string name)
{
    if (name.empty() || name.length() > 64)
        throw std::invalid_argument("Invalid station name");
    state_ = WAIT_FIRST_DATA;
    station_name_ = name;
    std::cerr << "PlayerComponent: now will play '" << name << "'" << std::endl;
}


void PlayerComponent::handle_event_(std::shared_ptr<Utility::Reactor::Event> event)
{
    if (event->name().substr(0, 16) == "/Lookup/Station/") {
        auto ev = std::dynamic_pointer_cast<Events::Lookup::StationEvent>(event);
        handle_station_event_(ev);

    } else if ("/Player/Internal/DataAvailable" == event->name()) {
        auto ev = std::dynamic_pointer_cast<Utility::Reactor::StreamEvent>(event);
        handle_data_(ev);
    }
}


void PlayerComponent::handle_station_event_(std::shared_ptr<Events::Lookup::StationEvent> event)
{
    auto bad_ev = std::dynamic_pointer_cast<Events::Lookup::StationTimedOutEvent>(event);
    if (bad_ev != nullptr) {
        // Station removed
        if (bad_ev->station_data().name == station_name_) {
            // Wombat is sad; wombat lost connection
            station_name_ = "";
            reactor_.broadcast_event(std::make_shared<ConnectionLostEvent>());
            socket_->leave_multicast(station_address_);
            station_address_ = Utility::Network::Address();
            std::cerr << "PlayerComponent: playied station timed out" << std::endl;
        }

    } else {
        // Station added or changed
        if (station_name_.empty())
            play_station(event->station_data().name); /* in either case we want to connect to it */

        if (station_name_ == event->station_data().name) {
            auto &station_data = event->station_data();
            // It is our station that we want to update
            if (!station_address_.empty())
                socket_->leave_multicast(station_address_);
            station_address_ = Utility::Network::Address(station_data.mcast_addr.host());
            socket_->bind_address(Utility::Network::Address::localhost(station_data.mcast_addr.port()));
            socket_->join_multicast(station_address_);
            std::cerr << "PlayerComponent: changed playied station addres to " << station_address_ << std::endl;
        }
    }
}


void PlayerComponent::handle_data_(std::shared_ptr<Utility::Reactor::StreamEvent> event)
{
    const size_t BUFFER_LENGTH = 1024;
    char *buffer = new char [BUFFER_LENGTH];
    size_t rd_len;

    while (socket_->read(buffer, BUFFER_LENGTH - 1, rd_len)) {
        buffer[rd_len] = 0;
        std::cout << buffer;
    }

    event->reenable_source();
    delete buffer;
}
