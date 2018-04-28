#include <iostream>
#include "TCPSocket.hpp"


int main()
{
    srand(static_cast<unsigned int>(time(nullptr)));
    TCPSocket socket(static_cast<uint16_t>(10000 + (rand() % 100)));
    std::clog << "INFO :  Listening on port " << socket.port() << std::endl;

    TCPConnection con = socket.accept();
    std::string greeting = "Hello World!\n";
    con.write(greeting.length(), greeting.c_str());
    con.close();

    return 0;
}