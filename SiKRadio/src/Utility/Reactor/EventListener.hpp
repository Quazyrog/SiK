#ifndef SIKRADIO_ABSTRACTEVENT_HPP
#define SIKRADIO_ABSTRACTEVENT_HPP


#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <list>
#include <condition_variable>
#include <atomic>
#include <regex>
#include "Event.hpp"



namespace Utility::Reactor {

/**
 * Asynchronous event listener for the generator.
 * It should have a worker-thread executing it's <c>operator()</c>, that handles incoming events. Incoming events are
 * queued in FIFO and handled by worker threads.
 */
class EventListener
{
private:
    /// Counts instances to give unique number to each; used by reactor to map id to listener object
    static std::atomic_uint32_t instances_counter_;

    /// Number of this instance
    uint32_t instance_id_ = ++instances_counter_;
    /// Should the worker threads be stopped?
    std::atomic_bool stopped_ = false;

    /// Guards access to queue of unhandled events
    std::mutex mutex_;
    /// Condition variable where workers wait for incoming events
    std::condition_variable for_event_;

    /// FIFO queue of unhandled events
    std::list<std::shared_ptr<Event>> unhandled_events_;
    /// Simple filters used to reject events that are not concerning this instance.
    /// For the sake of reactor's responsibility, they are filtered by worker threads.
    std::unordered_map<std::string, std::regex> filters_;

    /// Select next event to be handled next
    std::shared_ptr<Event> select_event_();


protected:
    /**
     * Add new filter that accepts names of events.
     * @note Note that filter time is linear to number of added filters
     * @param filter regex string that will be compiled to filter (ECMAScript syntax used in <c>std::regex</c>)
     */
    void add_filter_(const std::string &filter);
    /**
     * Remove filter accepting names of events.
     * @param filter same string that was given to <c>add_filter_()</c> that we want to remove
     */
    void remove_filter_(const std::string &filter);

    /**
     * Accept this event so it can be handled or reject (and won't be handled).
     * @param event event to be filtered
     * @return if the event was accepted
     */
    virtual bool filter_event_(std::shared_ptr<Event> event);
    /**
     * Handle event
     * @param event event that is handled (won't be null)
     */
    virtual void handle_event_(std::shared_ptr<Event> event) = 0;

public:
    /**
     * Return unique id of this instance
     */
    uint32_t instance_id() const
    {
        return instance_id_;
    }

    /**
     * Put given event into queue of awaiting events.
     * This is called from reactor, possibly in it's own thread.
     * All events are enqueued and filtering is done in worker threads (for the sake of reactor's responsibility).
     * @param event event that will be enqueued
     */
    void notify(std::shared_ptr<Event> event); // FIXME should the event be const?

    /**
     * Operate on this object as a working thread.
     * This will in loop wait for incoming events, filter and handle them, until <c>stop()</c> is called.
     */
    void operator()();
    /**
     * Stop threads working on this object so that they can leave <c>operator()</c> function.
     */
    void stop();
};

}

#endif //SIKRADIO_ABSTRACTEVENT_HPP
