#include <iostream>
#include "LookupComponent.hpp"
#include "Events.hpp"

using namespace Events::Lookup;



namespace {
const std::string EVENT_NAME_LOOKUP_TIME = "/Lookup/Internal/LookupTime";
const std::string EVENT_NAME_STATION_GC = "/Lookup/Internal/StationsGCTime";
const std::string EVENT_NAME_SOCKET = "/Lookup/Internal/Socket";
}


LookupComponent::LookupComponent(const Utility::Misc::Params &params, Utility::Reactor::Reactor &reactor):
    reactor_(reactor),
    discovery_addr_(params.discover_addr, params.ctrl_port)
{
    using namespace std::chrono_literals;

    // Socket
    ctrl_socket_ = std::make_shared<Utility::Network::UDPSocket>();
    ctrl_socket_->bind_address(Utility::Network::Address::localhost(params.ctrl_port));
    ctrl_socket_->enable_broadcast();
    ctrl_socket_->make_nonblocking();
    reactor_.add_resource(EVENT_NAME_SOCKET, ctrl_socket_);

    // Timers
    reactor_.add_resource(EVENT_NAME_LOOKUP_TIME, std::make_shared<Utility::Reactor::Timer>(0s, 5s));
    reactor_.add_resource(EVENT_NAME_STATION_GC, std::make_shared<Utility::Reactor::Timer>(20s, 2s));

    add_filter_("/Lookup/Internal/.*");
    add_filter_("/Player/WombatLooksForFriends");
    /* caller should add this object as listener */
}


void LookupComponent::handle_event_(std::shared_ptr<Utility::Reactor::Event> event)
{
    if (EVENT_NAME_LOOKUP_TIME == event->name()) {
        send_lookup_();

    } else if (EVENT_NAME_SOCKET == event->name()) {
        auto ev = std::dynamic_pointer_cast<Utility::Reactor::InputStreamEvent>(event);
        receive_ctrl_command_();
        ev->reenable_source();

    } else if (EVENT_NAME_STATION_GC == event->name()) {
        station_gc_();

    } else if ("/Player/WombatLooksForFriends" == event->name()) {
        player_wants_station_ = true;
    }
}


void LookupComponent::send_lookup_()
{
    const std::string DATA = "ZERO_SEVEN_COME_IN\n";
    std::cerr << "LookupComponent: Sending lookup request" << std::endl;
    ctrl_socket_->send(DATA.c_str(), DATA.length(), discovery_addr_);
}


void LookupComponent::receive_ctrl_command_()
{
    const size_t MAX_LEN = 1024;
    size_t rd_len, cmd_buffer_index = 0;
    bool drop_eol = false;

    auto read_buffer = new char [MAX_LEN];
    auto cmd_buffer = new char [MAX_LEN];
    memset(read_buffer, 0, MAX_LEN);
    memset(cmd_buffer, 0, MAX_LEN);

    while (ctrl_socket_->read(read_buffer, MAX_LEN, rd_len)) {
        for (size_t read_buffer_index = 0; read_buffer_index < rd_len; ++read_buffer_index) {
            if (read_buffer[read_buffer_index] == '\n') {
                // Command complete: send it for parser
                cmd_buffer[cmd_buffer_index] = 0;
                if (!drop_eol)
                    execute_ctrl_command_(std::stringstream(cmd_buffer));
                else
                    std::cerr << "LookupComponent: was too long; dropping" << std::endl;
                drop_eol = false;
                cmd_buffer_index = 0;

            } else {
                // Feed command buffer with next char
                drop_eol = (cmd_buffer_index == MAX_LEN - 1); /* we drop command if it already 1023 chars */
                if (drop_eol)
                    continue; /* [for] */
                cmd_buffer[cmd_buffer_index++] = read_buffer[read_buffer_index];
            }
        }
    }

    delete [] read_buffer;
    delete [] cmd_buffer;
}


void LookupComponent::execute_ctrl_command_(std::stringstream command)
{
    std::string cmdname;
    command >> cmdname;

    if (cmdname == "BOREWICZ_HERE") {
        // Read cmd params
        std::string address, station_name;
        uint16_t port;
        command >> address >> port;
        std::getline(command, station_name);
        station_name.erase(station_name.begin(), std::find_if(station_name.begin(), station_name.end(), [](int ch) {
            return !std::isspace(ch); })); /* that trims the string */
        if (station_name.length() > 64) {
            std::cerr << "LookupComponent: station name too long" << std::endl;
            return;
        }

        // Parse address
        Utility::Network::Address addr;
        try {
            addr = Utility::Network::Address(address, port);
            if (port == 0)
                throw std::invalid_argument("invalid port=0");
        } catch (std::invalid_argument &err) {
            std::cerr << "LookupComponent: station '" << station_name << "' IP invalid: " << err.what() << std::endl;
            return;
        }

        // Add or update station
        auto now = std::chrono::system_clock::now();
        auto station_it = stations_.find(station_name);
        if (station_it == stations_.end()) {
            // Add
            StationData data = {now, station_name, addr};
            std::cerr << "LookupComponent: new station '" << station_name << "' with address " << addr << std::endl;
            reactor_.broadcast_event(std::make_shared<NewStationEvent>(data));
            stations_[station_name] = data;

        } else {
            // Update
            station_it->second.last_reply = now;

            if (station_it->second.mcast_addr != addr) {
                // IP changed
                auto old = station_it->second.mcast_addr;
                station_it->second.mcast_addr = addr;
                std::cerr << "LookupComponent: station '" << station_name << "' changed address form ";
                std::cerr << old << " to " << station_it->second.mcast_addr << std::endl;
                reactor_.broadcast_event(std::make_shared<StationAddressChangedEvent>(station_it->second, old));

            } else if (player_wants_station_) {
                // No serious update, but we found new friend for Wombar
                player_wants_station_ = false;
                reactor_.broadcast_event(std::make_shared<WombatHereFriendEvent>(station_it->second));
            }
        }
    }
}


void LookupComponent::station_gc_()
{
    using std::chrono_literals::operator""s;
    size_t n_erased = 0;

    auto now = std::chrono::system_clock::now();
    for (auto it = stations_.begin(); it != stations_.end();) {
        if (now - it->second.last_reply > 20s) {
            std::cerr << "LookupComponent: station '" << it->second.name << "' timed out" << std::endl;
            reactor_.broadcast_event(std::make_shared<StationTimedOutEvent>(it->second));
            it = stations_.erase(it);
            ++n_erased;
        } else {
            ++it;
        }
    }
}
