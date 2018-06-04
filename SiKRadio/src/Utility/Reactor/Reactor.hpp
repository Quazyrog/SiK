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
#include "DescriptorResource.hpp"


namespace Utility::Reactor {

/**
 * The very heart of everything — asynchronous reactor.
 */ // TODO describe how it works
class Reactor
{
    static const std::string STOP_EVENT_NAME_;

    /// Shall the reactor's main loop be still running
    std::atomic_bool running_ = true;

    /// epoll object id
    int epoll_ = -1;
    /// pipe used to boardcast custom events
    int reactors_pipe_[2];

    /// Guards access to resources mappings
    std::mutex resources_lock_;
    /// Event names reserved for resources in epoll
    std::unordered_set<std::string> resources_names_;
    /// Holds mappings <c>descriptor -> resource</c> for all resources in epoll
    std::unordered_map<int, std::shared_ptr<DescriptorResource>> resources_;

    /// Guards access to event listeners map
    std::mutex listeners_guard_;
    /// Event listeners
    std::unordered_map<uint32_t, std::shared_ptr<EventListener>> listeners_;

    /// Guards access to custom events mappings
    std::mutex custom_events_lock_;
    /// Multimap <c>custom_event_name -> event_object</c>
    std::multimap<std::string, std::shared_ptr<Event>> custom_events_;

    /**
     * Change waited event mask for resource in epoll.
     * Assumes that we have <c>descriptor_resources_lock_</c> acquired.
     * @param resource altered resource
     * @param new_mask mask to be set
     */
    void change_event_mask_(const DescriptorResource *resource, uint32_t new_mask);

    /**
     * Actually does the work of <c>remove_resource()</c> but assumes that we have lock already acquired.
     * @param resource resource to be removed
     */
    void remove_resource_unsafe_(std::shared_ptr<DescriptorResource> resource);

    /**
     * Handles data incoming to Reactor's pipe.
     * Mostly used for broadcasting custom events as well as waking up the reactor after <c>stop()</c> is called.
     */
    void handle_pipe_();
    /**
     * Handles event from resource assigned to given descriptor.
     * @param fd descriptor where event occurred
     * @param events what kind of event occurred (as returned by <c>epoll_wait()</c>)
     */
    void handle_resource_(int fd, uint32_t events);
    /**
     * Send notification about event to all listeners.
     * It just places it in queue and wakes up one listener thread for every listener.
     * @param event event to be dispatched (#notnull)
     */
    void dispatch_(std::shared_ptr<Event> event);

    /**
     * Check if event assigned with that name is internal event.
     * Internal events are reads to <c>reactors_pipe_</c> without assigned event in <c>custom_events</c>, that are used
     * for waking up reactor's thread for some reason (for example when reactor was requested to stop).
     * Currently reserved internal event names are <c>"/Reactor/.*"</c>.
     * @param name tested event name — read from <c>reactors_pipe_</c>
     * @return if the event name is reserved for internal reactor's events
     */
    bool is_internal_name_(const std::string &name);
    /**
     * Handles internal reactor event; does following:
     *  - For <c>/Reactor/Stop</c> does nothing, as reactor probably was restarted
     * @param name name of internal event
     */
    void handle_internal_event_(const std::string &name);

public:
    /// Maximal length of event name
    static const size_t EVENT_NAME_MAX = PIPE_BUF;

    /**
     * Construct the reactor.
     * It will create all the necessary structures in OS.
     */
    Reactor();
    ~Reactor();

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
    void suspend_resource(const DescriptorResource *resource);
    /**
     * Reenable suspended resource.
     * Can be safely called on resources that were not suspended.
     * @param resource resource to be reenabled.
     */
    void reenable_resource(const DescriptorResource *resource);
    /**
     * Unbind <c>DescriptorResource</c> from this reactor.
     * Resource must be bound to the reactor.
     * @param resource resource to be unbound
     */
    void remove_resource(std::shared_ptr<DescriptorResource> resource);

    /**
     * Add asynchronous event listener to the reactor.
     * It will be notified about any event that occurred; single listener can be added just once.
     * @param listener new EventListener
     */
    void add_listener(std::shared_ptr<EventListener> listener);
    /**
     * Remove <c>EventListener</c> from reactor.
     * It must be present or else exception is thrown.
     * @param listener
     */
    void remove_listener(std::shared_ptr<EventListener> listener);

    /**
     * Broadcast custom event object with given name assigned.
     * Return immediately after event is passed to reactor, not waiting for broadcasting. Event name must be unused
     * (i.e. must not be reserved for internal reactor event nor for any resource).
     * @param event event that will be dispatched
     */
    void broadcast_event(std::shared_ptr<Event> event);

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
