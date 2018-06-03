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

class EventListener
{
private:
    std::atomic_bool stopped_ = false;

    std::mutex mutex_;
    std::condition_variable for_event_;

    std::list<std::shared_ptr<Event>> unhandled_events_;
    std::unordered_map<std::string, std::regex> filters_;

    std::shared_ptr<Event> select_event_();


protected:
    void add_filter_(const std::string &filter);
    void remove_filter_(const std::string &filter);

    virtual bool filter_event_(std::shared_ptr<Event> event);
    virtual void handle_event_(std::shared_ptr<Event> event) = 0;

public:
    virtual std::string name() const = 0;

    void notify(std::shared_ptr<Event> event);

    void start();
    void stop();
};

}

#endif //SIKRADIO_ABSTRACTEVENT_HPP
