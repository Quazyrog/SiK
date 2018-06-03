#ifndef SIKRADIO_REACTOR_HPP
#define SIKRADIO_REACTOR_HPP


#include <atomic>
#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <unordered_set>
#include <limits.h>
#include "EventListener.hpp"
#include "Timer.hpp"
#include "DescriptorResource.hpp"


namespace Utility::Reactor {
class Reactor
{
    /// Shall the reactor's main loop be still running
    std::atomic_bool running_ = true;

    /// epoll object id
    int epoll_ = -1;
    /// pipe used to boardcast custom events
    int reactors_pipe_[2];

    /// Guards access to resources mappings
    std::mutex descriptor_resources_lock_;
    /// Event names reserved for resources in epoll
    std::unordered_set<std::string> descriptor_resources_names_;
    /// Holds mappings <c>descriptor -> resource</c> for all resources in epoll
    std::unordered_map<int, std::shared_ptr<DescriptorResource>> descriptor_resources_;

    /// Guards access to event listeners map
    std::mutex listeners_guard_;
    /// Event listeners
    std::unordered_map<uint32_t, std::shared_ptr<EventListener>> listeners_;

    /**
     * Change waited event mask for resource in epoll.
     * Assumes that we have <c>descriptor_resources_lock_</c> acquired.
     * @param resource altered resource
     * @param new_mask mask to be set
     */
    void change_event_mask_(std::shared_ptr<const DescriptorResource> resource, uint32_t new_mask);

    /**
     * Actually does the work of <c>remove_resource()</c> but assumes that we have lock already acquired.
     * @param resource resource to be removed
     */
    void remove_resource_unsafe_(std::shared_ptr<DescriptorResource> resource);

    void handle_pipe_();
    void handle_resource_(int fd, uint32_t events);
    void dispatch_(std::shared_ptr<Event> event);

public:
    static const size_t EVENT_NAME_MAX = PIPE_BUF;

    Reactor();
    // FIXME destructor needed

    /**
     * Bind <c>DescriptorResource</c> to the reactor using given event name.
     * @param event_name Event name assigned to resource
     * @param resource new resource
     */
    void add_resource(const std::string &event_name, std::shared_ptr<DescriptorResource> resource);
    /**
     * Temporarily suspend all event handling for this resource.
     * This is particularly usefull (and bust be used) for suspending resources, that do not read data in their
     * <c>DescriptorResource::generate_event()</c> (such resources can produce events infinitely if not suspended).
     * @param resource
     */
    void suspend_resource(std::shared_ptr<const DescriptorResource> resource);
    /**
     * Reenable suspended resource.
     * Can be safely called on resources that were not suspended.
     * @param resource resource to be reenabled.
     */
    void reenable_resource(std::shared_ptr<const DescriptorResource> resource);
    /**
     * Unbind <c>DescriptorResource</c> from this reactor.
     * Resource must be bound to the reactor.
     * @param resource resource to be unbound
     */
    void remove_resource(std::shared_ptr<DescriptorResource> resource);

    void add_listener(std::shared_ptr<EventListener> listener);
    void remove_listener(std::shared_ptr<EventListener> listener);

    /**
     * Enter the main loop.
     * This waits on epoll for events and dispatches them until stop is called.
     */
    void operator()();
    /**
     * Stop reactor's main loop.
     * Awaiting events will not be dispatched.
     */
    void stop();
};

}



#endif //SIKRADIO_REACTOR_HPP
