#ifndef SIKRADIO_RECEIVERMISC_HPP
#define SIKRADIO_RECEIVERMISC_HPP

#include <Network/Address.hpp>
#include <chrono>



struct StationData
{
    std::chrono::system_clock::time_point last_reply;
    std::string name;
    Utility::Network::Address mcast_addr;
    Utility::Network::Address stat_addr;
};


#endif //SIKRADIO_RECEIVERMISC_HPP
