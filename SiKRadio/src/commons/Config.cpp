#include <Config.hpp>

namespace SiKRadio::Config {

Params defaults(int argc, char **argv)
{
    const unsigned int ALBUM = 382710;
    Params result;

    result.mcast_addr = "";
    result.discover_addr = "255.255.255.255";
    result.data_port = 20'000 + (ALBUM % 10'000);
    result.ctrl_port = 30'000 + (ALBUM % 10'000);
    result.ui_port = 10'000 + (ALBUM % 10'000);
    result.psize = 512; // bytes
    result.bsize = 64 * 1024; // bytes
    result.fsize = 128 * 1024; // bytes
    result.rtime = 250;  // milliseconds
    result.station_name = "Nienazwany Nadajnik";

    return result;
}

}
