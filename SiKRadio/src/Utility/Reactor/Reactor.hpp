#ifndef SIKRADIO_REACTOR_HPP
#define SIKRADIO_REACTOR_HPP


#include <atomic>
#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>

#include "Timer.hpp"
#include "DescriptorResource.hpp"


namespace Utility::Reactor {
class Reactor
{
    std::atomic_bool running_ = true;
    int epoll_ = -1;

    std::mutex descriptor_resources_lock_;
    std::unordered_map<std::string, std::shared_ptr<DescriptorResource>> descriptor_resources_;


public:
    Reactor();

    void add_descriptor_resource(const std::string &event_name, std::shared_ptr<DescriptorResource> resource);

    void operator()();
    void stop();
};

}



#endif //SIKRADIO_REACTOR_HPP
