#include <iostream>
#include <thread>
#include "TCPSocket.hpp"
#include "Telnet.hpp"
#include "RemoteTerminal.hpp"


int main()
{
    using std::chrono_literals::operator""ms;

    srand(static_cast<unsigned int>(time(nullptr)));
    TCPSocket socket(static_cast<uint16_t>(10000 + (rand() % 100)));
    std::clog << "MAIN  : Listening on port " << socket.port() << std::endl;

    auto con = RemoteTerminal(socket.accept());
    con.move(con.screenWidth() / 2 - 6, con.screenHeight() / 2);
    con << "Hello world!";
    con.flush();
    con.restoreConsole();
    con.close();

    return 0;
}