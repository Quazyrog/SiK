#include <iostream>
#include <thread>
#include "TCPSocket.hpp"
#include "Telnet.hpp"
#include "RemoteTerminal.hpp"
#include "MenuApplication.hpp"
#include "Util.hpp"


int main(int argc, char **argv)
{
    if (argc < 2) {
        std::cerr << "USAGE: " << argv[0] << " {PORT_NUMBER}" << std::endl;
        return 1;
    }
    long unsigned int portnum = strtoul(argv[1], nullptr, 10);
    if (portnum < 1 || portnum > 65535) {
        std::cerr << "Invalid port number" << std::endl;
        return 1;
    }

    TCPSocket *socket = nullptr;
    try {
        socket = new TCPSocket(static_cast<uint16_t>(portnum));
    } catch (SystemError &e) {
        std::cerr << "Failed to bind the port: " << e.what() << std::endl;
        return 1;
    }
    std::clog << "MAIN  : Listening on port " << socket->port() << std::endl;

    while (true) {
        try {
            MenuApplication menus(socket->accept());
            menus.run();
        } catch (std::runtime_error &e) {
            std::clog << "MAIN: error occured: " << e.what() << std::endl;
        }
    }
}