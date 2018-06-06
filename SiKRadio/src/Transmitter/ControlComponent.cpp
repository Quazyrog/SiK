#include <Misc.hpp>
#include <Reactor/Reactor.hpp>
#include <Network/UDPSocket.hpp>
#include "ControlComponent.hpp"



ControlComponent::ControlComponent(const Utility::Misc::Params &params, Utility::Reactor::Reactor &reactor):
    reactor_(reactor),
    name_(params.station_name)
{
    socket_ = std::make_shared<Utility::Network::UDPSocket>();
    socket_->make_nonblocking();
    socket_->bind_address(Utility::Network::Address::localhost(params.ctrl_port));
    reactor_.add_resource("/Control/Internal/Socket", socket_);

    add_filter_("/Control/Internal/.*");
}


void ControlComponent::handle_event_(std::shared_ptr<Utility::Reactor::Event> event)
{
    if ("/Control/Internal/Socket" == event->name()) {
        auto ev = std::dynamic_pointer_cast<Utility::Reactor::StreamEvent>(event);
        handle_data_(ev);
    }
}


void ControlComponent::handle_data_(std::shared_ptr<Utility::Reactor::StreamEvent> event)
{
    const size_t MAX_LEN = 1024;
    size_t rd_len;
    char *buff = new char [MAX_LEN];
    Utility::Network::Address addr;
    socket_->receive(buff, MAX_LEN - 1, rd_len, addr);
    buff[rd_len] = 0;
    execute_lookup_command_(std::stringstream(buff), addr);
    delete [] buff;
    event->reenable_source();
}


void ControlComponent::execute_lookup_command_(std::stringstream ss, Utility::Network::Address from_address)
{
    std::string cmd;
    ss >> cmd;

    if ("ZERO_SEVEN_COME_IN" == cmd) {
        std::string hello = std::string("BOREWICZ_HERE ") + from_address.host() + " "
                + std::to_string(from_address.port()) + " " + name_ + "\n";
        socket_->send(hello.c_str(), hello.length(), from_address);
    }
}
