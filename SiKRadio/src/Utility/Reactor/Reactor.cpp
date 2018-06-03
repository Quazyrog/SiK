#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <cassert>

#include "Reactor.hpp"
#include "../Exceptions.hpp"

using Utility::Exceptions::SystemError;

namespace Utility::Reactor {

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
    std::lock_guard lock_guard(descriptor_resources_lock_);
    if (event_name.substr(0, 9) == "/Reactor/")
        throw std::invalid_argument("Event names prefixed `/Reactor/` are reserved");

    // Check if mappings are free and bind resource
    if (descriptor_resources_names_.find(event_name) != descriptor_resources_names_.end())
        throw std::invalid_argument("Resource with event's name `" + event_name + "` already present");
    if (descriptor_resources_.find(resource->descriptor()) != descriptor_resources_.end())
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
    descriptor_resources_[resource->descriptor()] = resource;
    descriptor_resources_names_.emplace(event_name);
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
}


void Reactor::suspend_resource(std::shared_ptr<const DescriptorResource> resource)
{
    std::lock_guard lg(descriptor_resources_lock_);
    change_event_mask_(resource, 0);
}


void Reactor::reenable_resource(std::shared_ptr<const DescriptorResource> resource)
{
    std::lock_guard lg(descriptor_resources_lock_);
    change_event_mask_(resource, resource->event_mask());
}


void Reactor::change_event_mask_(std::shared_ptr<const DescriptorResource> resource, uint32_t new_mask)
{
    // Assure we have such resource

    auto it = descriptor_resources_.find(resource->descriptor());
    if (it == descriptor_resources_.end())
        throw std::invalid_argument("No such resource in reactor");
    assert(it->second == resource);

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
    auto fd_it = descriptor_resources_.find(resource->descriptor());
    if (fd_it == descriptor_resources_.end())
        throw std::invalid_argument("No such resource in reactor");
    assert(fd_it->second == resource);
    descriptor_resources_.erase(fd_it);

    // Free name
    auto nm_it = descriptor_resources_names_.find(resource->bound_name());
    assert(nm_it != descriptor_resources_names_.end());
    descriptor_resources_names_.erase(nm_it);

    resource->unbind_();
}


void Reactor::handle_pipe_()
{
    // TODO implement custom events broadcasting
}


void Reactor::handle_resource_(int fd, uint32_t events)
{
    DescriptorResource::ResourceAction action;

    // Generate the event
    descriptor_resources_lock_.lock();
    auto resource = descriptor_resources_.at(fd);
    auto event = resource->generate_event(events, action); // yep, we still want lock
    descriptor_resources_lock_.unlock();

    // Dispatch
    if (event != nullptr) { // resource can decide to ignore that event
        assert(event->name() == resource->bound_name());
        dispatch_(event);
    }

    // Handle action
    descriptor_resources_lock_.lock();
    if (!resource->is_bound_to(this))
        return; // somebody removed the resource... hope he knew what he was doing xD
    if (action & DescriptorResource::SUSPEND) {
        change_event_mask_(resource, 0);
    }
    if (action & DescriptorResource::REMOVE_FROM_REACTOR) {
        remove_resource_unsafe_(resource);
    }
    descriptor_resources_lock_.unlock();
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
    std::lock_guard lg(descriptor_resources_lock_);
    remove_resource_unsafe_(resource);
}


Reactor::~Reactor()
{
    close(epoll_);
    close(reactors_pipe_[0]);
    close(reactors_pipe_[1]);
    // RAII in resources will do the rest
}

}
