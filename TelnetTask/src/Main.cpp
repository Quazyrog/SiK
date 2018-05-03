#include <iostream>
#include "TCPSocket.hpp"
#include "Telnet.hpp"


int main()
{
    srand(static_cast<unsigned int>(time(nullptr)));
    TCPSocket socket(static_cast<uint16_t>(10000 + (rand() % 100)));
    std::clog << "MAIN:  Listening on port " << socket.port() << std::endl;

    auto con = Telnet::Connection(socket.accept());
    con << "Hello World!\n";
    con << Telnet::IAC << Telnet::DO << Telnet::LINEMODE;
    con << Telnet::IAC << Telnet::SB << Telnet::LINEMODE << "\x01\x00" << Telnet::IAC << Telnet::SE;
    con << Telnet::IAC << Telnet::WILL << Telnet::SUPPRESS_GO_AHEAD;
    con << Telnet::IAC << Telnet::WILL << Telnet::ECHO;
    con.flush();
    char c = con.get();
    std::cout << (int)c;
    con.close();

    return 0;
}