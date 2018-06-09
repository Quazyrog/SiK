#include <iostream>
#include <thread>
#include <csignal>

#include <boost/program_options.hpp>
#include <boost/program_options.hpp>
#include <boost/log/attributes/attribute.hpp>
#include <boost/log/attributes/constant.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

#include <Network/UDPSocket.hpp>
#include <Misc.hpp>
#include <Reactor/Reactor.hpp>
#include "ControlComponent.hpp"
#include "SpellCasterComponent.hpp"
#include "TransmitterMisc.hpp"


namespace {

Utility::Reactor::Reactor *reactor;


Utility::Misc::Params parse_args(int argc, char **argv)
{
    using namespace boost::program_options;
    Utility::Misc::Params params;

    // Declare the supported options.
    options_description desc("Allowed options");
    desc.add_options()
            ("help,h", "produce help message")
            ("mcast-addr,a", value(&(params.mcast_addr)), "transmitter multi-cast address")
            ("data-port,d", value(&(params.data_port)), "transmitter multi-cast port")
            ("ctrl-port,C", value(&(params.ctrl_port)), "control port (udp)")
            ("fsize,f", value(&(params.fsize)), "FIFO size (bytes)")
            ("psize,p", value(&(params.psize)), "package size (bytes)")
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
    auto control_component = std::make_shared<ControlComponent>(params, *reactor, lookup_logger);
    reactor->add_listener(control_component);

    // And then the wizard was born
    Utility::Misc::LoggerType arinvald_logger;
    arinvald_logger.add_attribute("Component", boost::log::attributes::constant<std::string>("Arinvald"));
    auto arinvald = std::make_shared<SpellCasterComponent>(params, *reactor, arinvald_logger);
    reactor->add_listener(arinvald);

    // Main loops
    std::thread lookup_component_thread{[control_component](){control_component->operator()();}};
    std::thread no_a_single_word_of_truth{[control_component](){control_component->operator()();}};
    reactor->operator()();

    // Shut down
    control_component->stop();
    arinvald->stop();
    lookup_component_thread.join();
    no_a_single_word_of_truth.join();

    return 0;
}

