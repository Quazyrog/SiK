#include <iostream>
#include "LookupComponent.hpp"
#include "Events.hpp"

using namespace Events::Lookup;



namespace {
const std::string EVENT_NAME_LOOKUP_TIME = "/Lookup/Internal/LookupTime";
const std::string EVENT_NAME_STATION_GC = "/Lookup/Internal/StationsGCTime";
const std::string EVENT_NAME_SOCKET = "/Lookup/Internal/Socket";
}


LookupComponent::LookupComponent(const Utility::Misc::Params &params, Utility::Reactor::Reactor &reactor,
                                 Utility::Misc::LoggerType logger):
    logger_(logger),
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
    add_filter_("/Player/Retransmission");
    LOG_INFO(logger_) << "initialization complete";
}


void LookupComponent::handle_event_(std::shared_ptr<Utility::Reactor::Event> event)
{
    if (EVENT_NAME_LOOKUP_TIME == event->name()) {
        send_lookup_();

    } else if (EVENT_NAME_SOCKET == event->name()) {
        auto ev = std::dynamic_pointer_cast<Utility::Reactor::StreamEvent>(event);
        receive_ctrl_command_();
        ev->reenable_source();

    } else if (EVENT_NAME_STATION_GC == event->name()) {
        station_gc_();

    } else if ("/Player/WombatLooksForFriends" == event->name()) {
        player_wants_station_ = true;
    } else if ("/Player/Retransmission" == event->name()) {
        auto ev = std::dynamic_pointer_cast<Events::Player::RetransmissionEvent>(event);
        std::stringstream ss;
        ss << "LOUDER_PLEASE ";
        for (auto pn : ev->packets_list())
            ss << pn << ",";
        auto str = ss.str();
        str[str.length() - 1] = '\n';
        std::cerr << "LookupComponent: to " << ev->station_address() << " send " << str;
        ctrl_socket_->send(str.c_str(), str.length(), ev->station_address());
    }
}


void LookupComponent::send_lookup_()
{
    const std::string DATA = "ZERO_SEVEN_COME_IN\n";
    LOG_DEBUG(logger_) << "sending lookup request";
    ctrl_socket_->send(DATA.c_str(), DATA.length(), discovery_addr_);
}


void LookupComponent::receive_ctrl_command_()
{
    const size_t MAX_LEN = 1024;
    size_t rd_len;
    auto read_buffer = new char [MAX_LEN];
    memset(read_buffer, 0, MAX_LEN);
    Utility::Network::Address from;

    while (ctrl_socket_->receive(read_buffer, MAX_LEN, rd_len, from)) {
        read_buffer[rd_len] = 0;
        execute_ctrl_command_(std::stringstream(read_buffer), from);
    }

    delete [] read_buffer;
}


void LookupComponent::execute_ctrl_command_(std::stringstream command, Utility::Network::Address from)
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
            LOG_WARNING(logger_) << "LookupComponent: station name too long `" << station_name << "`";
            return;
        }

        // Parse address
        Utility::Network::Address addr;
        try {
            addr = Utility::Network::Address(address, port);
            if (port == 0)
                throw std::invalid_argument("invalid port=0");
        } catch (std::invalid_argument &err) {
            LOG_WARNING(logger_) << "station '" << station_name << "' IP invalid: " << err.what();
            return;
        }

        // Add or update station
        auto now = std::chrono::system_clock::now();
        auto station_it = stations_.find(station_name);
        if (station_it == stations_.end()) {
            // Add
            StationData data = {now, station_name, addr, from};
            LOG_INFO(logger_) << "new station '" << station_name << "' with mcast=" << addr << " addr=" << from;
            reactor_.broadcast_event(std::make_shared<NewStationEvent>(data));
            stations_[station_name] = data;

        } else {
            // Update
            station_it->second.last_reply = now;

            if (station_it->second.mcast_addr != addr || station_it->second.stat_addr != from) {
                // IP changed
                auto old = station_it->second.mcast_addr;
                station_it->second.mcast_addr = addr;
                station_it->second.stat_addr = from;
                LOG_INFO(logger_) << "station '" << station_name << "' changed address form " << old << " to "
                                  << station_it->second.mcast_addr << std::endl;
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
            LOG_INFO(logger_) << "LookupComponent: station '" << it->second.name << "' timed out";
            reactor_.broadcast_event(std::make_shared<StationTimedOutEvent>(it->second));
            it = stations_.erase(it);
            ++n_erased;
        } else {
            ++it;
        }
    }
}
