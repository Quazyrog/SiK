#include <iostream>
#include <thread>
#include "Reactor.hpp"



int main()
{
    using namespace std::chrono_literals;

    Reactor reactor;
    std::thread reactor_thread([&reactor]() {
        reactor.add_timer(1000, 1000, "1 second");
        reactor.add_timer(500, 2000, "2 seconds");
        reactor.add_timer(250, 2000, "4 seconds");
        reactor.add_timer(1, 333, ".333 second");
        reactor.add_timer(1000, 0, "Later");
    });

    std::cout << "Hello, World!" << std::endl;
    reactor();
    reactor_thread.join();
    return 0;
}