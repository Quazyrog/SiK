#include <thread>
#include <csignal>
#include <iostream>

#include <boost/program_options.hpp>
#include <boost/log/attributes/attribute.hpp>
#include <boost/log/attributes/constant.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

#include <Network/UDPSocket.hpp>
#include <Misc.hpp>
#include <Reactor/Reactor.hpp>
#include "LookupComponent.hpp"
#include "PlayerComponent.hpp"



namespace {

Utility::Reactor::Reactor *reactor;


Utility::Misc::Params parse_args(int argc, char **argv)
{
    using namespace boost::program_options;
    Utility::Misc::Params params;
    params.station_name = "";

    // Declare the supported options.
    options_description desc("Allowed options");
    desc.add_options()
            ("help,h", "produce help message")
            ("discover-addr,d", value(&(params.discover_addr)), "receiver discovery multi-cast address")
            ("ctrl-port,C", value(&(params.ctrl_port)), "control port (udp)")
            ("ui-port,U", value(&(params.ui_port)), "telnet-cli port (tcp)")
            ("bsize,b", value(&(params.bsize)), "buffer size (bytes)")
            ("rtime,R", value(&(params.rtime)), "retransmission wait time (milliseconds)")
            ("station-name,n", value(&(params.station_name)), "name of station to play")
            ("verbose,v", value(&(params.verbosity)), "use if you like spam on stderr — verbosity level");

    variables_map vm;
    store(parse_command_line(argc, argv, desc), vm);
    notify(vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
        exit(0);
    }

    // FIXME ,,Graceful shutdown''
    return params;
}


void sigint_handler(int signum)
{
    std::cerr << "Stopped with signal " << signum << std::endl;
    if (reactor != nullptr)
        reactor->stop();
}

}


int main(int argc, char **argv)
{
    // Initialization things
    auto params = parse_args(argc, argv);
    reactor = new Utility::Reactor::Reactor();
    signal(SIGINT, sigint_handler);
    Utility::Misc::init_boost_log(params);

    // Construct lookup component
    Utility::Misc::LoggerType lookup_logger;
    lookup_logger.add_attribute("Component", boost::log::attributes::constant<std::string>("Lookup"));
    auto lookup_component = std::make_shared<LookupComponent>(params, *reactor, lookup_logger);
    reactor->add_listener(lookup_component);

    // Player component
    Utility::Misc::LoggerType player_logger;
    player_logger.add_attribute("Component", boost::log::attributes::constant<std::string>("Player"));
    auto player_component = std::make_shared<PlayerComponent>(params, *reactor, player_logger);
    if (!params.station_name.empty())
        player_component->play_station(params.station_name);
    reactor->add_listener(player_component);

    // Main loops
    std::thread lookup_component_thread{[lookup_component](){lookup_component->operator()();}};
    std::thread player_component_thread{[player_component](){player_component->operator()();}};
    reactor->operator()();

    // Shut down
    lookup_component->stop();
    player_component->stop();
    lookup_component_thread.join();
    player_component_thread.join();

    return 0;
}

