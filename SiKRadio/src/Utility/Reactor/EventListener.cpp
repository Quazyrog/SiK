#include <regex>
#include "EventListener.hpp"
#include "Reactor.hpp"



namespace Utility::Reactor {

void EventListener::add_filter_(std::regex filter)
{
    filters_.insert(std::move(filter));
}


void EventListener::remove_filter_(const std::regex &filter)
{
    auto it = filters_.find(filter);
    if (it != filters_.end())
        filters_.erase(it);
}


void EventListener::notify(std::shared_ptr<Event> event)
{
    std::lock_guard lock(mutex_);
    unhandled_events_.push_back(event);
    for_event_.notify_one();
}


void EventListener::start()
{
    while (!stopped_) {
        auto event = select_event_();
        if (event != nullptr)
            handle_event_(event);
    }
}


void EventListener::stop()
{
    stopped_ = true;
    for_event_.notify_all();
}


std::shared_ptr<Event> EventListener::select_event_()
{
    std::shared_ptr<Event> selected;
    std::unique_lock lock(mutex_, std::defer_lock);

    do {
        lock.lock();
        for_event_.wait(lock, []() {return !unhandled_events_.empty() | stopped_;});
        if (stopped_) {
            lock.unlock();
            return nullptr;
        }
        selected = unhandled_events_.back();
        unhandled_events_.pop_back();
        lock.unlock();

        if (!filter_event_(selected))
            selected = nullptr;
    } while (selected != nullptr);

    return selected;
}


bool EventListener::filter_event_(std::shared_ptr<Event> event)
{
    const std::string &name = event->name();
    for (auto filter : filters_) {
        if (std::regex_match(name, filter))
            return true;
    }
    return false;
}

}
