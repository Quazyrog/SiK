#include <iostream>
#include <boost/program_options.hpp>
#include <Network/UDPSocket.hpp>
#include <Misc.hpp>
#include <Reactor/Reactor.hpp>
#include <thread>
#include <csignal>
#include "LookupComponent.hpp"



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
            ("discover-addr,d", value(&(params.discover_addr)), "receiver discovery multi-cast address")
            ("ctrl-port,C", value(&(params.ctrl_port)), "control port (udp)")
            ("ui-port,U", value(&(params.ui_port)), "telnet-cli port (tcp)")
            ("bsize,b", value(&(params.bsize)), "buffer size (bytes)")
            ("rtime,R", value(&(params.rtime)), "retransmission wait time (milliseconds)")
            ("station-name,n", value(&(params.discover_addr)), "name of station to play");

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
    auto params = parse_args(argc, argv);
    reactor = new Utility::Reactor::Reactor();
    signal(SIGINT, sigint_handler);

    // Construct lookup component
    auto lookup_component = std::make_shared<LookupComponent>(params, *reactor);
    reactor->add_listener(lookup_component);

    // Main loops
    std::thread lookup_component_thread{[lookup_component](){lookup_component->operator()();}};
    reactor->operator()();

    // Shut down
    lookup_component->stop();
    lookup_component_thread.join();

    return 0;
}

