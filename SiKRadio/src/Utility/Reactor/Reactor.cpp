#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <cassert>

#include "Reactor.hpp"
#include "../Exceptions.hpp"

using Utility::Exceptions::SystemError;

namespace Utility::Reactor {

const std::string Reactor::STOP_EVENT_NAME_ = "/Reactor/Stop";


Reactor::Reactor()
{
    // Create epoll
    epoll_ = epoll_create1(0);
    if (epoll_ < 0)
        throw SystemError("Unable to create Reactor's epool");

    // Create custom events pipe...
    if (pipe2(reactors_pipe_, O_DIRECT) != 0) {
        close(epoll_);
        throw SystemError("Unable to create Reactor's Pipe");
    }

    // ...and add it to poll
    epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = reactors_pipe_[0];
    if (epoll_ctl(epoll_, EPOLL_CTL_ADD, reactors_pipe_[0], &ev) != 0) {
        close(epoll_);
        close(reactors_pipe_[0]);
        close(reactors_pipe_[1]);
        throw SystemError("Failed to add Reactor's Pipe to epoll");
    }
}


void Reactor::add_resource(const std::string &event_name, std::shared_ptr<DescriptorResource> resource)
{
    std::lock_guard lock_guard(resources_lock_);
    if (is_internal_name_(event_name))
        throw std::invalid_argument("Event names prefixed `/Reactor/` are reserved");

    // Check if mappings are free and bind resource
    if (resources_names_.find(event_name) != resources_names_.end())
        throw std::invalid_argument("Resource with event's name `" + event_name + "` already present");
    if (resources_.find(resource->descriptor()) != resources_.end())
        throw std::invalid_argument("Resource with fd " + std::to_string(resource->descriptor()) + " already present");
    resource->bind_(this, event_name);

    // Add to epoll
    epoll_event ev;
    ev.events = resource->event_mask();
    ev.data.fd = resource->descriptor();
    if (epoll_ctl(epoll_, EPOLL_CTL_ADD, resource->descriptor(), &ev) != 0) {
        resource->unbind_();
        throw SystemError("Failed to add resource " + event_name + " to epoll");
    }

    // Now add mappings
    resources_[resource->descriptor()] = resource;
    resources_names_.emplace(event_name);
}


void Reactor::operator()()
{
    running_ = true;

    while (running_) {
        // Wait fot event
        epoll_event ev;
        epoll_wait(epoll_, &ev, 1, -1);

        if (!running_)
            break;
        if (ev.data.fd == reactors_pipe_[0])
            handle_pipe_();
        else
            handle_resource_(ev.data.fd, ev.events);
    }
}


void Reactor::stop()
{
    running_ = false;
    write(reactors_pipe_[1], STOP_EVENT_NAME_.c_str(), STOP_EVENT_NAME_.length());
}


void Reactor::suspend_resource(const DescriptorResource *resource)
{
    std::lock_guard lg(resources_lock_);
    change_event_mask_(resource, 0);
}


void Reactor::reenable_resource(const DescriptorResource *resource)
{
    std::lock_guard lg(resources_lock_);
    change_event_mask_(resource, resource->event_mask());
}


void Reactor::change_event_mask_(const DescriptorResource *resource, uint32_t new_mask)
{
    // Assure we have such resource

    auto it = resources_.find(resource->descriptor());
    if (it == resources_.end())
        throw std::invalid_argument("No such resource in reactor");
    assert(it->second.get() == resource);

    // Now change mask
    epoll_event ev;
    ev.events = new_mask;
    ev.data.fd = resource->descriptor();
    if (epoll_ctl(epoll_, EPOLL_CTL_MOD, resource->descriptor(), &ev) != 0)
        throw SystemError("Failed to modify resource mask fd=" + std::to_string(resource->descriptor()));
}


void Reactor::remove_resource_unsafe_(std::shared_ptr<DescriptorResource> resource)
{
    // Erase from descriptors map
    auto fd_it = resources_.find(resource->descriptor());
    if (fd_it == resources_.end())
        throw std::invalid_argument("No such resource in reactor");
    assert(fd_it->second == resource);
    resources_.erase(fd_it);

    // Free name
    auto nm_it = resources_names_.find(resource->bound_name());
    assert(nm_it != resources_names_.end());
    resources_names_.erase(nm_it);

    resource->unbind_();
}


void Reactor::handle_pipe_()
{
    // Read event name
    char *buffer = new char[EVENT_NAME_MAX + 1];
    std::memset(buffer, 0, EVENT_NAME_MAX + 1);
    /* pipe is in packet (O_DIRECT) mode, thus it will read entire name and only entire name*/
    read(reactors_pipe_[0], buffer, EVENT_NAME_MAX);
    std::string event_name{buffer};
    delete [] buffer;
    buffer = nullptr;

    // Dispatch or handle internal
    if (is_internal_name_(event_name)) {
        handle_internal_event_(event_name);
    } else {
        custom_events_lock_.lock();
        auto range = custom_events_.equal_range(event_name);
        assert(range.first != range.second);
        auto event = range.first->second;
        custom_events_.erase(range.first);
        custom_events_lock_.unlock();
        dispatch_(event);
    }

}


void Reactor::handle_resource_(int fd, uint32_t events)
{
    DescriptorResource::ResourceAction action;

    // Generate the event
    resources_lock_.lock();
    auto resource = resources_.at(fd);
    auto event = resource->generate_event(events, action); // yep, we still want lock
    resources_lock_.unlock();

    // Dispatch
    if (event != nullptr) { // resource can decide to ignore that event
        assert(event->name() == resource->bound_name());
        dispatch_(event);
    }

    // Handle action
    resources_lock_.lock();
    if (!resource->is_bound_to(this))
        return; // somebody removed the resource... hope he knew what he was doing xD
    if (action & DescriptorResource::SUSPEND) {
        change_event_mask_(resource.get(), 0);
    }
    if (action & DescriptorResource::REMOVE_FROM_REACTOR) {
        remove_resource_unsafe_(resource);
    }
    resources_lock_.unlock();
}


void Reactor::dispatch_(std::shared_ptr<Event> event)
{
    for (auto it : listeners_) {
        auto listener = it.second;
        listener->notify(event);
    }
}


void Reactor::add_listener(std::shared_ptr<EventListener> listener)
{
    std::lock_guard lg(listeners_guard_);
    if (listeners_.find(listener->instance_id()) != listeners_.end())
        throw std::invalid_argument("Listener with id " + std::to_string(listener->instance_id()) + " added twice");
    listeners_[listener->instance_id()] = listener;
}


void Reactor::remove_listener(std::shared_ptr<EventListener> listener)
{
    auto it = listeners_.find(listener->instance_id());
    if (it == listeners_.end())
        throw std::invalid_argument("No such listener in Reactor with id " + std::to_string(listener->instance_id()));
    listeners_.erase(it);
}


void Reactor::remove_resource(std::shared_ptr<DescriptorResource> resource)
{
    std::lock_guard lg(resources_lock_);
    remove_resource_unsafe_(resource);
}


Reactor::~Reactor()
{
    close(epoll_);
    close(reactors_pipe_[0]);
    close(reactors_pipe_[1]);
    // RAII in resources will do the rest
}


void Reactor::broadcast_event(std::shared_ptr<Event> event)
{
    const std::string &event_name = event->name();
    // Maybe name too long (writer/reads in packet mode lower than PIPE_BUF are atomic according to manual)
    if (event_name.length() > EVENT_NAME_MAX)
        throw std::invalid_argument("Too long name in Reactor::broadcast_event; max=" + std::to_string(EVENT_NAME_MAX));
    if (event_name != event->name())
        throw std::invalid_argument("`event_name` and `event->name()` disagree");

    // Check if event name is unused
    resources_lock_.lock();
    if (is_internal_name_(event_name))
        throw std::invalid_argument("Event names prefixed `/Reactor/` are reserved");
    if (resources_names_.find(event_name) != resources_names_.end())
        throw std::invalid_argument("Resource with event's name `" + event_name + "` already present");
    resources_lock_.unlock();

    // Put mapping
    custom_events_lock_.lock();
    custom_events_.insert({event_name, event});
    custom_events_lock_.unlock();

    // Now push notification with pipe
    if (write(reactors_pipe_[1], event_name.c_str(), event_name.length()) != event_name.length())
        throw SystemError("Could not write to pipe");
}


bool Reactor::is_internal_name_(const std::string &name)
{
    return name.substr(0, 9) == "/Reactor/";
}


void Reactor::handle_internal_event_(const std::string &name)
{
    /* Now there is one internal event and ish should be ignored */
    assert(name == STOP_EVENT_NAME_);
}

}
