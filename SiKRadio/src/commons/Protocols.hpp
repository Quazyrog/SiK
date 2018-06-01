#ifndef SIKRADIO_PROTOCOLS_HPP
#define SIKRADIO_PROTOCOLS_HPP

#include <stdint-gcc.h>



namespace Protocols {

struct StationData
{
    std::string name;
    std::string mcast_addr;
    uint16_t port_num;
};

namespace Audio {

struct AutioDataPacket
{
    uint64_t session_id;
    uint64_t first_byte_num;
    uint8_t audio_data[];
};

}

}

#endif //SIKRADIO_PROTOCOLS_HPP
