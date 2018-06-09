#include <iostream>
#include <cassert>
#include <climits>
#include <Reactor/Reactor.hpp>
#include <Exceptions.hpp>
#include <Reactor/Timer.hpp>
#include "PlayerComponent.hpp"

using namespace Events::Player;



PlayerComponent::PlayerComponent(const Utility::Misc::Params &params, Utility::Reactor::Reactor &reactor,
                                 Utility::Misc::LoggerType logger):
    logger_(logger),
    reactor_(reactor),
    buffer_(params.bsize)
{
    using namespace std::chrono_literals;

    // Socket
    socket_ = std::make_shared<Utility::Network::UDPSocket>();
    socket_->make_nonblocking();
    reactor_.add_resource("/Player/Internal/DataAvailable", socket_);

    // Stdout
    stdout_ = Utility::Reactor::StreamResource::stdout_resource();
    stdout_->make_nonblocking();
    reactor_.add_resource("/Player/Internal/StdoutReady", stdout_);

    timer_ = std::make_shared<Utility::Reactor::Timer>(10'000, params.rtime * 1'000);
    timer_->stop();
    reactor_.add_resource("/Player/Internal/Retransmit", timer_);

    // Filters
    add_filter_("/Player/Internal/.*");
    add_filter_("/Lookup/Station/.*");
    LOG_INFO(logger_) << "initialization complete";
}


void PlayerComponent::play_station(std::string name)
{
    if (name.length() > 64)
        throw std::invalid_argument("Invalid station name");
    station_name_ = name;
    LOG_INFO(logger_) << "now will play '" << name << "'";
}


void PlayerComponent::handle_event_(std::shared_ptr<Utility::Reactor::Event> event)
{
    if (event->name().substr(0, 16) == "/Lookup/Station/") {
        auto ev = std::dynamic_pointer_cast<Events::Lookup::StationEvent>(event);
        handle_station_event_(ev);

    } else if ("/Player/Internal/DataAvailable" == event->name()) {
        auto ev = std::dynamic_pointer_cast<Utility::Reactor::StreamEvent>(event);
        handle_data_(ev);

    } else if ("/Player/Internal/StdoutReady" == event->name()) {
        stdout_ready_ = true;
        try_write_();

    } else if ("/Player/Internal/Retransmit" == event->name()) {
        LOG_DEBUG(logger_) << "wants retransmission from " << station_ctrl_addr_;
        reactor_.broadcast_event(std::make_shared<RetransmissionEvent>(buffer_.missing_list(), station_ctrl_addr_));
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
            LOG_WARNING(logger_) << "played station timed out";
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
            station_ctrl_addr_ = event->station_data().stat_addr;

            // Now try to connect to station
            try {
                auto new_addr = Utility::Network::Address::localhost(station_data.mcast_addr.port());
                if (new_addr != local_address_)
                    socket_->bind_address(station_data.mcast_addr);
                socket_->join_multicast(station_data.mcast_addr);
                local_address_ = new_addr;
                station_address_ = station_data.mcast_addr;
            } catch (Utility::Exceptions::SystemError &err) {
                LOG_ERROR(logger_) << "Cannot connect to station '" << station_name_ << "' on mcast "
                        << station_address_.host() << " and port " << station_data.mcast_addr.port() << ";"
                        << " error: " << err.what();
                play_station("");
                return;
            }
            LOG_INFO(logger_) << "now connected to '" << station_name_ << "' with address " << station_address_;
        }
    }
}


void PlayerComponent::handle_data_(std::shared_ptr<Utility::Reactor::StreamEvent> event)
{
    const size_t BUFFER_SIZE = std::numeric_limits<uint16_t>::max();
    char *buffer = new char [BUFFER_SIZE];

    size_t rd_len = 0;
    socket_->read(buffer, BUFFER_SIZE, rd_len);
    if (rd_len == 0) {
        LOG_WARNING(logger_) << "that's quite unexpected... packet with no data";
        return;
    }
    try {
        AudioBuffer::Packet pk = AudioBuffer::Packet::from_data(buffer, rd_len);
        // FIXME session id handling
        buffer_.put(pk);
        if (!timer_->runing())
            timer_->start();
    } catch (std::invalid_argument &err) {
        LOG_WARNING(logger_) << "cannot load data to audio packet: " << err.what();
    }

    if (stdout_ready_)
        try_write_();

    event->reenable_source();
    /* We receive only one packet to make receive/write simultaneous */
}


void PlayerComponent::try_write_()
{
    if (!buffer_.filled_with_magic() || !stdout_ready_)
        return;

    AudioBuffer::Packet packet;
    if (buffer_.load_head(packet)) {
    // Buffer is not empty, we can write
        if (packet.first_byte_num() - last_written_data_ > packet.audio_size())
            LOG_WARNING(logger_) << "skipped " << last_written_data_ << " to " << packet.first_byte_num() - 1
                                 << " (" << packet.first_byte_num() - last_written_data_ << " bytes)";

        size_t wr_len;
        stdout_->write(packet.audio_data() + partial_write_, packet.audio_size(), wr_len);
        partial_write_ += wr_len;
        last_written_data_ = packet.first_byte_num() + partial_write_;

        // Check if entire pack was written, pop head
        assert(partial_write_ <= packet.audio_size());
        if (partial_write_ == packet.audio_size()) {
            partial_write_ = 0;
            buffer_.pop_head();
        } else {
            /* Write was partial -> stdout is not ready */
            stdout_ready_ = false;
            reactor_.reenable_resource(stdout_.get());
        }

    } else {
    // Buffer empty, would print empty data
        timer_->stop();
        buffer_.clear();

    }
}
