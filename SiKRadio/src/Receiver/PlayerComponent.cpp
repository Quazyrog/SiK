#include <iostream>
#include <cassert>
#include <Reactor/Reactor.hpp>
#include <Exceptions.hpp>
#include <Reactor/Timer.hpp>
#include "PlayerComponent.hpp"

using namespace Events::Player;

namespace {
const size_t MAX_PACKET_SIZE = 256 * 256;
}


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
    state_ = WAIT_FIRST_DATA;
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
        reactor_.broadcast_event(std::make_shared<RetransmissionEvent>(buffer_.retransmit_list(), station_ctrl_addr_));
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
    using Utility::AudioPacket;

    const size_t metadata_len = offsetof(Utility::AudioPacket, audio_data);
    char *buffer = new char [MAX_PACKET_SIZE];
    bool read_ready = true;

    do {
        size_t rd_len = 0;
        AudioPacket packet;
        read_ready = socket_->read(buffer, MAX_PACKET_SIZE, rd_len);
        if (rd_len == 0)
            continue;
        packet.session_id = Utility::Misc::hton<uint64_t>(
                *reinterpret_cast<uint64_t*>(buffer + offsetof(AudioPacket, session_id)));
        packet.first_byte_num = Utility::Misc::hton<uint64_t>(
                *reinterpret_cast<uint64_t*>(buffer + offsetof(AudioPacket, first_byte_num)));
        packet.audio_data = buffer + metadata_len;
        if (state_ == WAIT_FIRST_DATA) {
            state_ = WAIT_BUFFER;
            timer_->start();
            try {
                buffer_.reset(packet.first_byte_num, rd_len - metadata_len);
            } catch (std::invalid_argument &err) {
                LOG_ERROR(logger_) << "cannot reset buffer: " << err.what();
                goto HELL;
            }
        }
        try {
            buffer_.put(packet);
        } catch (Utility::BufferStorageError &err) {
            LOG_WARNING(logger_) << "cannot store buffer data: " << err.what();
            goto HELL;
        }
HELL:
        try_write_();
    } while (read_ready);

    event->reenable_source();
}


void PlayerComponent::try_write_()
{
    if (state_ == WAIT_FIRST_DATA)
        return;
    if (state_ == WAIT_BUFFER) {
        if (buffer_.is_filled_with_magic())
            state_ = STREAM;
        else
            return;
    }

    Utility::AudioPacket packet;
    packet.audio_data = new char [buffer_.packet_data_size()];
    LOG_DEBUG(logger_) << "stdouting data";

    try {
        while (stdout_ready_) {
            buffer_ >> packet; /* Consume packet, this may throw breaking the loop if no packet data avail */

            size_t wr_len;
            stdout_ready_ = stdout_->write(packet.audio_data, buffer_.packet_data_size(), wr_len);
        }

        // We exited from loop not by exception, so stdout is full, we should again listen for it to be ready
        reactor_.reenable_resource(stdout_.get());
    } catch (Utility::BufferStorageError &err) {
        LOG_DEBUG(logger_) << "buffer empty: should reconnect";
        buffer_.clear();
        state_ = WAIT_FIRST_DATA;
    }
}
