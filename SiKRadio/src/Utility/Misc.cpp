#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/sources/severity_feature.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/attributes/attribute.hpp>
#include <boost/log/attributes/constant.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

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


void init_boost_log(const Utility::Misc::Params &params)
{
    using namespace boost::log;
    add_common_attributes();
    auto sink = add_console_log(std::clog, keywords::format = "[%TimeStamp% %Component%]: %Message%");
    core::get()->add_sink(sink);
    if (params.verbosity == 0)
        core::get()->set_filter(trivial::severity >= trivial::info);
    else if (params.verbosity == 1)
        core::get()->set_filter(trivial::severity >= trivial::debug);
    else
        core::get()->set_filter(trivial::severity >= trivial::trace);
}

}
