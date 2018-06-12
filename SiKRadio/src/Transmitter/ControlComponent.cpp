#include <Misc.hpp>
#include <Reactor/Reactor.hpp>
#include <Network/UDPSocket.hpp>
#include <Reactor/Timer.hpp>
#include "ControlComponent.hpp"
#include "Events.hpp"


ControlComponent::ControlComponent(const Utility::Misc::Params &params, Utility::Reactor::Reactor &reactor,
                                   Utility::Misc::LoggerType logger):
    reactor_(reactor),
    name_(params.station_name),
    logger_(logger)
{
    socket_ = std::make_shared<Utility::Network::UDPSocket>();
    socket_->make_nonblocking();
    socket_->bind_address(Utility::Network::Address::localhost(params.ctrl_port));
    reactor_.add_resource("/Control/Internal/Socket", socket_);

    mcast_addr_ = Utility::Network::Address(params.mcast_addr, params.data_port);

    auto retrans_timer = std::make_shared<Utility::Reactor::Timer>(params.rtime * 1'000, params.rtime * 1'000);
    reactor_.add_resource("/Control/Internal/Retransmission", retrans_timer);

    add_filter_("/Control/Internal/.*");

    LOG_INFO(logger_) << "initialization complete";
}


void ControlComponent::handle_event_(std::shared_ptr<Utility::Reactor::Event> event)
{
    if ("/Control/Internal/Socket" == event->name()) {
        auto ev = std::dynamic_pointer_cast<Utility::Reactor::StreamEvent>(event);
        handle_data_(ev);

    } else if ("/Control/Internal/Retransmission" == event->name()) {
        reactor_.broadcast_event(std::make_shared<RetransmissionEvent>(std::move(retrans_packets_)));
        retrans_packets_.clear();
    }
}


void ControlComponent::handle_data_(std::shared_ptr<Utility::Reactor::StreamEvent> event)
{
    const size_t MAX_LEN = 16192;
    size_t rd_len;
    char *buff = new char [MAX_LEN];
    Utility::Network::Address addr;

    socket_->receive(buff, MAX_LEN - 1, rd_len, addr);
    if (buff[rd_len - 1] == '\n') {
        buff[rd_len - 1] = 0;
        LOG_DEBUG(logger_) << "executing control command `" << buff << "`";
        execute_command_(std::stringstream(buff), addr);
    } else {
        LOG_DEBUG(logger_) << " invalid (wrong terminated) control command ignored";
    }

    delete [] buff;
    event->reenable_source();
}


void ControlComponent::execute_command_(std::stringstream ss, Utility::Network::Address from_address)
{
    std::string cmd;
    ss >> cmd;

    if ("ZERO_SEVEN_COME_IN" == cmd) {
        LOG_DEBUG(logger_) << "Lookup requested from " << static_cast<std::string>(from_address);
        std::string hello = std::string("BOREWICZ_HERE ") + mcast_addr_.host() + " " 
                            + std::to_string(mcast_addr_.port()) + " " + name_;
        socket_->send(hello.c_str(), hello.length(), from_address);

    } else if (cmd == "LOUDER_PLEASE") {
        std::string packets;
        std::getline(ss, packets);
        std::regex full("^ \\d+(,\\d+)*$");
        std::regex sub("\\d+");
        std::smatch packet_nr;
        if (!std::regex_match(packets, full))
            return;
        std::regex_search(packets, packet_nr, sub);
        for (auto match : packet_nr)
            retrans_packets_.insert(std::stoull(match, nullptr, 10));
    }
}
