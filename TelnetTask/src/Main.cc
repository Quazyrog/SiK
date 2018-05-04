#include <iostream>
#include <thread>
#include "TCPSocket.hpp"
#include "Telnet.hpp"
#include "RemoteTerminal.hpp"
#include "MenuApplication.hpp"


int main()
{
    srand(static_cast<unsigned int>(time(nullptr)));
    TCPSocket socket(static_cast<uint16_t>(10000 + (rand() % 100)));
    std::clog << "MAIN  : Listening on port " << socket.port() << std::endl;

    while (true) {
        try {
            MenuApplication menus(socket.accept());
            menus.run();
        } catch (std::runtime_error &e) {
            std::clog << "MAIN: error occured: " << e.what() << std::endl;
        }
    }
}