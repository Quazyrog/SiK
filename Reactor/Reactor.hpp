#ifndef REACTOR_REACTOR_HPP
#define REACTOR_REACTOR_HPP

#include <unordered_map>
#include <atomic>



class Reactor
{
    std::atomic_bool running = true;
    int epoll = -1;

    std::unordered_map<int, std::string> timer_names;
    std::unordered_map<std::string, int> timer_fds;

public:
    Reactor();

    void add_timer(unsigned int rel_start_ms, unsigned int interval_ms, std::string name);

    void operator()();

    void stop();
};



#endif //REACTOR_REACTOR_HPP
