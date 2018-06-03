#include "Event.hpp"
#include "Reactor.hpp"



namespace Utility::Reactor {

Event::Event(std::string name):
    name_(std::move(name))
{
    if (!Reactor::validate_event_name(name, false))
        throw std::invalid_argument("invalid event name `" + name + "`");
}

}