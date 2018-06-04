#include <Misc.hpp>

namespace Utility::Misc {

Params::Params()
{
    const unsigned int ALBUM = 382710;

    mcast_addr = "";
    discover_addr = "255.255.255.255";
    data_port = 20'000 + (ALBUM % 10'000);
    ctrl_port = 30'000 + (ALBUM % 10'000);
    ui_port = 10'000 + (ALBUM % 10'000);
    psize = 512; // bytes
    bsize = 64 * 1024; // bytes
    fsize = 128 * 1024; // bytes
    rtime = 250;  // milliseconds
    station_name = "Nienazwany Nadajnik";
}

}
