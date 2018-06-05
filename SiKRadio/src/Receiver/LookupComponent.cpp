#include <iostream>
#include "LookupComponent.hpp"


namespace {
const std::string EVENT_NAME_TIMER_EXPIRED = "/Lookup/TimerExpired";
const std::string EVENT_NAME_SOCKET = "/Lookup/Socket";
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

    // Timer
    lookup_timer_ = std::make_shared<Utility::Reactor::Timer>(0s, 5s);
    reactor_.add_resource(EVENT_NAME_TIMER_EXPIRED, lookup_timer_);

    add_filter_("/Lookup/.*");
    /* caller should add this object as listener */
}


void LookupComponent::handle_event_(std::shared_ptr<Utility::Reactor::Event> event)
{
    if (event->name() == EVENT_NAME_TIMER_EXPIRED) {
        std::cerr << "LookupComponent: Sending lookup request" << std::endl;
        send_lookup_();
    } else if (event->name() == EVENT_NAME_SOCKET) {
        std::cerr << "LookupComponent: Some data arrived" << std::endl;
        auto ev = std::dynamic_pointer_cast<Utility::Reactor::InputStreamEvent>(event);
        receive_ctrl_command_();
        ev->reenable_source();
    }
}


void LookupComponent::send_lookup_()
{
    const std::string DATA = "ZERO_SEVEN_COME_IN\n";
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
                if (!drop_eol) {
                    std::cerr << "LookupComponent: command is `" << cmd_buffer << "`" << std::endl;
                    execute_ctrl_command_(std::stringstream(cmd_buffer));
                } else {
                    std::cerr << "LookupComponent: was too long; dropping" << std::endl;
                }
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
}


void LookupComponent::execute_ctrl_command_(std::stringstream command)
{
    std::string cmdname;
    command >> cmdname;
    if (cmdname == "BOREWICZ_HERE") {
        std::string address, station_name;
        uint16_t port;
        command >> address >> port;
        std::getline(command, station_name);
        std::cerr << "LookupComponent: reply from " << station_name << " :: " << address << ":" << port << std::endl;
    }
}
