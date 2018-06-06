#include <iostream>
#include <cassert>
#include <Reactor/Reactor.hpp>
#include "PlayerComponent.hpp"

using namespace Events::Player;



PlayerComponent::PlayerComponent(const Utility::Misc::Params &params, Utility::Reactor::Reactor &reactor):
    reactor_(reactor),
    buffer_(params.bsize, params.psize - sizeof(Utility::AudioPacket) + sizeof(char *))
{
    in_packet_.audio_data = new char [buffer_.packet_data_size()];
    std::memset(in_packet_.audio_data, 0, buffer_.packet_data_size());

    // Socket
    socket_ = std::make_shared<Utility::Network::UDPSocket>();
    socket_->make_nonblocking();
    socket_->enable_broadcast();
    reactor_.add_resource("/Player/Internal/DataAvailable", socket_);

    // Stdout
    stdout_ = Utility::Reactor::StreamResource::stdout_resource();
    stdout_->make_nonblocking();
    reactor_.add_resource("/Player/Internal/StdoutReady", stdout_);

    // Filters
    add_filter_("/Player/Internal/.*");
    add_filter_("/Lookup/Station/.*");
}


void PlayerComponent::play_station(std::string name)
{
    if (name.empty() || name.length() > 64)
        throw std::invalid_argument("Invalid station name");
    state_ = WAIT_FIRST_DATA;
    station_name_ = name;
    part_packet_read_ = 0;
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

    } else if ("/Player/Internal/StdoutReady" == event->name()) {
        stdout_ready_ = true;
        try_write_();
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
    const size_t metadata_len = offsetof(Utility::AudioPacket, audio_data);
    bool read_ready = true;

    do {
        size_t rd_len = 0;
        assert(part_packet_read_ < buffer_.packet_size());

        if (part_packet_read_ < metadata_len) {
            // Reading metadata
            read_ready = socket_->read(reinterpret_cast<char *>(&in_packet_ + part_packet_read_),
                    metadata_len - part_packet_read_, rd_len);
            part_packet_read_ += rd_len;
            assert(part_packet_read_ <= metadata_len);

            if (part_packet_read_ == metadata_len) {
                // Metadata read complete
                assert(state_ == WAIT_FIRST_DATA);
                in_packet_.session_id = Utility::Misc::hton(in_packet_.session_id);
                in_packet_.first_byte_num = Utility::Misc::hton(in_packet_.first_byte_num);
                if (!buffer_.was_reset()) {
                    state_ = WAIT_BUFFER;
                    buffer_.reset(in_packet_.first_byte_num);
                }
            }

        } else {
            // Reading data
            assert(part_packet_read_ < buffer_.packet_size());
            char * const offset = in_packet_.audio_data + part_packet_read_ - metadata_len;
            const size_t max_read = buffer_.packet_data_size() - (offset - in_packet_.audio_data);
            read_ready = socket_->read(offset, max_read, rd_len);
            assert(part_packet_read_ <= buffer_.packet_size());

            if (part_packet_read_ == buffer_.packet_size()) {
                // Entire packet was read
                try {
                    buffer_.put(in_packet_);
                } catch (Utility::BufferStorageError &err) {
                    std::cerr << "PlayerComponent: cannot store received packet (first_byte=";
                    std::cerr << std::to_string(Utility::Misc::hton(in_packet_.first_byte_num)) << ") ";
                    std::cerr << "error: " << err.what() << std::endl;
                }

                part_packet_read_ = 0;
                try_write_();
            }
        }
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

    try {
        while (stdout_ready_) {
            assert(part_packet_write_ <= buffer_.packet_data_size());

            if (part_packet_write_ == buffer_.packet_data_size()) {
                // Start writing new packet
                buffer_ >> out_packet_; /* Consume packet, this may throw breaking the loop if no packet data avail */
                part_packet_write_ = 0;
            }

            assert(part_packet_write_ < buffer_.packet_data_size());
            size_t wr_len;
            stdout_ready_ = stdout_->write(out_packet_.audio_data + part_packet_write_,
                    buffer_.packet_data_size() - part_packet_write_, wr_len);
            part_packet_write_ = wr_len;
            assert(part_packet_write_ <= buffer_.packet_data_size());
        }

        // We exited from loop not by exception, so stdout is full, we should again listen for it to be ready
        reactor_.reenable_resource(stdout_.get());
    } catch (Utility::BufferStorageError &err) {
        std::cerr << "PLayerComponent: Buffer empty: should reconnect" << std::endl;
        buffer_.clear();
        state_ = WAIT_FIRST_DATA;
    }
}
