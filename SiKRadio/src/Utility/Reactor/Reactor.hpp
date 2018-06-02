#ifndef SIKRADIO_REACTOR_HPP
#define SIKRADIO_REACTOR_HPP


#include <atomic>
#include <string>
#include <unordered_map>
#include <memory>

#include "Timer.hpp"



namespace Utility::Reactor {
class Reactor
{
    std::atomic_bool running_ = true;
    int epoll_ = -1;
    std::unordered_map<std::string, std::string> descriptor_resources;


public:
    static bool validate_event_name(const std::string &name, bool allow_wildcard);


    Reactor();

    void add_descriptor_resource(const std::string &event_name, std::shared_ptr<DescriptorResource> resource);

    void operator()();
    void stop();
};

}



#endif //SIKRADIO_REACTOR_HPP
