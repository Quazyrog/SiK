#include <Network/UDPSocket.hpp>



int main(int argc, char **argv)
{
    using namespace Utility::Network;
    UDPSocket sock;
    sock.enable_broadcast();
    sock.join_multicast(Address("239.10.12.1"));
    sock.bind_address(Address::localhost(4242));
    char buf[111];
    size_t xd;
    sock.read(buf, 111, xd);
    std::cout << buf << std::endl;
    return 0;
}
